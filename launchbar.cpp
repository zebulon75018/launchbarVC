#include <QDialogButtonBox>

#include "launchbar.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QScreen>
#include <QApplication>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QProcess>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QFileInfo>
#include <QSettings>
#include <QDir>
#include <QRegExp>
#include <QContextMenuEvent>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLineEdit>
#include <QLabel>
#include <QDialog>
#include <QTabWidget>
#include <QSlider>
#include <QColorDialog>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QMap>
#include <QDebug>
#include <cmath>

// Pour Qt5, QMimeDatabase peut nécessiter un include explicite
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    #include <QMimeDatabase>
    #include <QMimeType>
#endif

// LaunchButton Implementation
LaunchButton::LaunchButton(const QString &iconPath, const QString &program, 
                           const QJsonArray &children, const QString &label,
                           int iconSize, QWidget *parent)
    : QPushButton(parent), m_program(program), m_iconPath(iconPath), 
      m_label(label), m_children(children), m_scale(1.0), m_buttonIndex(-1), 
      m_isHovered(false), m_iconSize(iconSize)
{
    setFixedSize(m_iconSize, m_iconSize);
    setFlat(true);
    setCursor(Qt::PointingHandCursor);
    setAcceptDrops(hasChildren());
    setToolTip(label);
    
    m_iconPixmap = QPixmap(iconPath);
    if (m_iconPixmap.isNull()) {
        m_iconPixmap = QPixmap(":/images/icon.png");
        //m_iconPixmap = QPixmap(m_iconSize, m_iconSize);
        //m_iconPixmap.fill(Qt::gray);
    }
    
    m_animation = new QPropertyAnimation(this, "scale", this);
    m_animation->setDuration(200);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);
}

void LaunchButton::setIconPath(const QString &iconPath)
{
    m_iconPath = iconPath;
    m_iconPixmap = QPixmap(iconPath);
    if (m_iconPixmap.isNull()) {
        m_iconPixmap = QPixmap(m_iconSize, m_iconSize);
        m_iconPixmap.fill(Qt::gray);
    }
    update();
}

void LaunchButton::setScale(qreal scale)
{
    m_scale = scale;
    update();
}

void LaunchButton::enterEvent(QEvent *event)
{
    QPushButton::enterEvent(event);
    m_isHovered = true;
    m_animation->stop();
    m_animation->setStartValue(m_scale);
    m_animation->setEndValue(1.5);
    m_animation->start();
    update();
}

void LaunchButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    m_isHovered = false;
    m_animation->stop();
    m_animation->setStartValue(m_scale);
    m_animation->setEndValue(1.0);
    m_animation->start();
    update();
}

void LaunchButton::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    int size = m_iconSize * m_scale;
    int offset = (m_iconSize - size) / 2;
    
    QRect iconRect(offset, offset, size, size);
    painter.drawPixmap(iconRect, m_iconPixmap.scaled(size, size, 
                       Qt::KeepAspectRatio, Qt::SmoothTransformation));
    
    if (!m_label.isEmpty() && hasChildren()) {
        QFont font = painter.font();
        font.setPointSize(8);
        font.setBold(true);
        painter.setFont(font);

        painter.setPen(QColor(0, 0, 0));
        QRect textRect = rect().adjusted(6, m_iconSize - 12, -2, -2);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, m_label);
        
        painter.setPen(m_isHovered ? Qt::white : QColor(200, 200, 200));
        textRect = rect().adjusted(2, m_iconSize - 16, -2, -2);
        painter.drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, m_label);

        // Draw a little triangle
        QPainterPath path;
        // Set pen to this point.
        path.moveTo (10, 0);
        // Draw line from pen point to this point.
        path.lineTo (0, 0);

        //path.moveTo (endPointX1, endPointY1); // <- no need to move
        path.lineTo (0,   10);

        //path.moveTo (endPointX2,   endPointY2); // <- no need to move
        path.lineTo (10, 0);

        painter.setPen (Qt :: NoPen);
        painter.fillPath (path, QBrush (QColor ("lightgrey")));

    }
    
    if (!m_program.isEmpty() && !m_label.isEmpty() && !hasChildren()) {
        QFileInfo fileInfo(m_program);
        if (!fileInfo.isExecutable()) {
            QString fileName = fileInfo.fileName();
            if (fileName.length() > 10) {
                fileName = fileName.left(8) + "..";
            }

            QFont font = painter.font();
            font.setPointSize(7);
            painter.setFont(font);

            painter.setPen( Qt::black);
            QRect textRect = rect().adjusted(6, m_iconSize - 12, -2, -2);
            painter.drawText(textRect, Qt::AlignCenter, fileName);

            painter.setPen(m_isHovered ? Qt::white : Qt::lightGray);
            textRect = rect().adjusted(2, m_iconSize - 16, -2, -2);
            painter.drawText(textRect, Qt::AlignCenter, fileName);
        }
    }
}

void LaunchButton::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    
    QAction *editAction = menu.addAction("Propriétés");
    QAction *removeAction = menu.addAction("Supprimer");
    
    QAction *selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == editAction) {
        emit editRequested(this);
    } else if (selectedAction == removeAction) {
        emit removeRequested(this);
    }
}

void LaunchButton::dragEnterEvent(QDragEnterEvent *event)
{
    if (hasChildren() && event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void LaunchButton::dropEvent(QDropEvent *event)
{
    if (!hasChildren()) return;
    
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urls = mimeData->urls();
        if (!urls.isEmpty()) {
            QString filePath = urls.first().toLocalFile();
            emit dropOnCategory(this, filePath);
            event->acceptProposedAction();
        }
    }
}

// LaunchBar Implementation
LaunchBar::LaunchBar(QWidget *parent)
    : QWidget(parent), m_position(Bottom), m_dragging(false), 
      m_configPath("config.json"), m_opacity(200), m_backgroundColor(40, 40, 40),
      m_iconSize(64)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock);
    setAcceptDrops(true);
    
    // Chemins par défaut pour les icônes
    m_iconPaths = QStringList() << "/usr/share/icons/hicolor/48x48/apps/"<< "/usr/share/icons/hicolor/64x64/apps/"<< "/usr/share/icons/hicolor/128x128/apps/"<< "/usr/share/pixmaps/" ;
    
    m_layout = new QHBoxLayout(this);
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(5);
    
    m_subButtonsWidget = new QWidget();
    m_subButtonsWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    m_subButtonsWidget->setAttribute(Qt::WA_TranslucentBackground);
    m_subButtonsWidget->hide();
    
    loadConfig();
    updatePosition();
}

void LaunchBar::loadConfig(const QString &configPath)
{
    m_configPath = configPath;
    QFile file(configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Impossible d'ouvrir le fichier de configuration:" << configPath;
        m_items = QJsonArray();
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();
    
    if (!doc.isObject()) {
        qWarning() << "Format JSON invalide";
        return;
    }
    
    QJsonObject root = doc.object();
    
    if (root.contains("position")) {
        QString pos = root["position"].toString().toLower();
        if (pos == "top") setPosition(Top);
        else if (pos == "left") setPosition(Left);
        else if (pos == "right") setPosition(Right);
        else setPosition(Bottom);
    }
    
    if (root.contains("items") && root["items"].isArray()) {
        m_items = root["items"].toArray();
        clearButtons();
        createButtons(m_items);
    }
    
    if (root.contains("opacity")) {
        m_opacity = root["opacity"].toInt(200);
    }
    if (root.contains("backgroundColor")) {
        QJsonObject bgColor = root["backgroundColor"].toObject();
        m_backgroundColor = QColor(
            bgColor["r"].toInt(40),
            bgColor["g"].toInt(40),
            bgColor["b"].toInt(40)
        );
    }
    if (root.contains("iconSize")) {
        m_iconSize = root["iconSize"].toInt(64);
    }
    if (root.contains("iconPaths") && root["iconPaths"].isArray()) {
        m_iconPaths.clear();
        QJsonArray paths = root["iconPaths"].toArray();
        for (const QJsonValue &val : paths) {
            m_iconPaths.append(val.toString());
        }
    }
    
    adjustSize();
    updatePosition();
}

void LaunchBar::saveConfig()
{
    QJsonObject root;
    
    QString posStr = "bottom";
    switch (m_position) {
        case Top: posStr = "top"; break;
        case Left: posStr = "left"; break;
        case Right: posStr = "right"; break;
        default: posStr = "bottom"; break;
    }
    root["position"] = posStr;
    root["items"] = m_items;
    root["opacity"] = m_opacity;
    
    QJsonObject bgColor;
    bgColor["r"] = m_backgroundColor.red();
    bgColor["g"] = m_backgroundColor.green();
    bgColor["b"] = m_backgroundColor.blue();
    root["backgroundColor"] = bgColor;
    
    root["iconSize"] = m_iconSize;
    
    QJsonArray iconPathsArray;
    for (const QString &path : m_iconPaths) {
        iconPathsArray.append(path);
    }
    root["iconPaths"] = iconPathsArray;
    
    QJsonDocument doc(root);
    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Impossible de sauvegarder la configuration:" << m_configPath;
        return;
    }
    
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    qDebug() << "Configuration sauvegardée dans" << m_configPath;
}

void LaunchBar::createButtons(const QJsonArray &items)
{
    int index = 0;
    for (const QJsonValue &value : items) {
        if (!value.isObject()) continue;
        QString icon;
        
        QJsonObject item = value.toObject();
        QString program = item["program"].toString();
        QFileInfo fi = QFileInfo(program);
        if (fi.isDir()) {
         icon = ":/images/folder.png";
        } else {
         icon = item["icon"].toString();
        }
        QString label = item["label"].toString();
        QJsonArray children;
        
        if (item.contains("category") && item["category"].isArray()) {
            children = item["category"].toArray();
        }
        
        LaunchButton *btn = new LaunchButton(icon, program, children, label, m_iconSize, this);
        btn->setButtonIndex(index);
        btn->setJsonPath({index}); // Chemin à la racine
        connect(btn, &QPushButton::clicked, this, &LaunchBar::onButtonClicked);
        connect(btn, &LaunchButton::removeRequested, this, &LaunchBar::removeButton);
        connect(btn, &LaunchButton::editRequested, this, &LaunchBar::editButton);
        connect(btn, &LaunchButton::dropOnCategory, this, &LaunchBar::handleDropOnCategory);
        m_layout->addWidget(btn);
        m_buttons.append(btn);
        index++;
    }
}

void LaunchBar::onButtonClicked()
{
    LaunchButton *btn = qobject_cast<LaunchButton*>(sender());
    if (!btn) return;
    
    if (btn->hasChildren()) {
        // Vérifier si ce bouton a déjà un sous-menu ouvert
        if (m_buttonToSubMenu.contains(btn)) {
            // Le sous-menu est déjà ouvert, le fermer (toggle)
            QWidget *existingSubMenu = m_buttonToSubMenu[btn];
            closeChildMenus(existingSubMenu); // Fermer les enfants de ce sous-menu
            m_subMenuParents.remove(existingSubMenu);
            m_buttonToSubMenu.remove(btn);
            existingSubMenu->deleteLater();
        } else {
            // Le sous-menu n'est pas ouvert, l'ouvrir
            // Mais d'abord, fermer les sous-menus frères (au même niveau)
            QWidget *btnParent = btn->parentWidget();
            closeChildMenus(btnParent);
            // Afficher le nouveau sous-menu
            showSubButtons(btn->children(), btn);
        }
    } else if (!btn->program().isEmpty()) {
        // Lancer le programme sans fermer les menus
        QProcess::startDetached(btn->program());
    }
}

void LaunchBar::showSubButtons(const QJsonArray &children, LaunchButton *parent)
{
    // Créer un nouveau widget pour ce sous-menu
    QWidget *subMenuWidget = new QWidget();
    subMenuWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    subMenuWidget->setAttribute(Qt::WA_TranslucentBackground);
    
    // Enregistrer le parent de ce widget
    QWidget *parentWidget = parent->parentWidget();
    m_subMenuParents[subMenuWidget] = parentWidget;
    
    // Enregistrer l'association bouton -> sous-menu
    m_buttonToSubMenu[parent] = subMenuWidget;
    
    // Déterminer l'orientation du layout (perpendiculaire au parent)
    QBoxLayout *parentLayout = qobject_cast<QBoxLayout*>(parentWidget->layout());
    QBoxLayout *newLayout = createPerpendicularLayout(parentLayout);
    newLayout->setContentsMargins(10, 10, 10, 10);
    newLayout->setSpacing(5);
    
    // Ajouter un fond au widget
    QString bgColor = QString("background-color: rgba(%1, %2, %3, %4); border-radius: 15px;")
        .arg(m_backgroundColor.red())
        .arg(m_backgroundColor.green())
        .arg(m_backgroundColor.blue())
        .arg(m_opacity);
    subMenuWidget->setStyleSheet(bgColor);
    
    subMenuWidget->setLayout(newLayout);
    
    // Obtenir le chemin JSON du parent
    QList<int> parentPath = parent->jsonPath();
    
    // Créer les boutons pour ce sous-menu
    int childIndex = 0;
    for (const QJsonValue &value : children) {
        if (!value.isObject()) continue;
        
        QJsonObject item = value.toObject();
        QString icon = item["icon"].toString();
        QString program = item["program"].toString();
        QString label = item["label"].toString();
        QJsonArray subChildren;
        
        if (item.contains("category") && item["category"].isArray()) {
            subChildren = item["category"].toArray();
        }
        
        LaunchButton *btn = new LaunchButton(icon, program, subChildren, label, m_iconSize, subMenuWidget);
        
        // Définir le chemin JSON de ce bouton
        QList<int> btnPath = parentPath;
        btnPath.append(childIndex);
        btn->setJsonPath(btnPath);
        
        // Connecter pour permettre les clics (programmes ou sous-catégories)
        connect(btn, &QPushButton::clicked, this, &LaunchBar::onButtonClicked);
        connect(btn, &LaunchButton::removeRequested, this, &LaunchBar::removeButton);
        connect(btn, &LaunchButton::editRequested, this, &LaunchBar::editButton);
        connect(btn, &LaunchButton::dropOnCategory, this, &LaunchBar::handleDropOnCategory);
        newLayout->addWidget(btn);
        childIndex++;
    }
    
    // Positionner le widget par rapport au parent
    subMenuWidget->adjustSize();
    QPoint parentPos = parent->mapToGlobal(QPoint(0, 0));
    
    // Déterminer la position selon l'orientation
    if (qobject_cast<QHBoxLayout*>(parentLayout)) {
        // Parent horizontal -> afficher verticalement
        if (parent->mapToGlobal(QPoint(0, 0)).y() > QApplication::primaryScreen()->geometry().height() / 2) {
            // En haut
            subMenuWidget->move(parentPos.x(), parentPos.y() - subMenuWidget->height() - 10);
        } else {
            // En bas
            subMenuWidget->move(parentPos.x(), parentPos.y() + parent->height() + 10);
        }
    } else {
        // Parent vertical -> afficher horizontalement
        if (parent->mapToGlobal(QPoint(0, 0)).x() > QApplication::primaryScreen()->geometry().width() / 2) {
            // À gauche
            subMenuWidget->move(parentPos.x() - subMenuWidget->width() - 10, parentPos.y());
        } else {
            // À droite
            subMenuWidget->move(parentPos.x() + parent->width() + 10, parentPos.y());
        }
    }
    
    // Afficher le widget
    subMenuWidget->show();
}

void LaunchBar::closeChildMenus(QWidget *parentWidget)
{
    // Trouver tous les sous-menus qui sont enfants de ce parent
    QList<QWidget*> toRemove;
    
    for (auto it = m_subMenuParents.begin(); it != m_subMenuParents.end(); ++it) {
        if (it.value() == parentWidget) {
            QWidget *childWidget = it.key();
            toRemove.append(childWidget);
            // Fermer aussi récursivement les enfants de cet enfant
            closeChildMenus(childWidget);
        }
    }
    
    // Supprimer les widgets trouvés
    for (QWidget *widget : toRemove) {
        m_subMenuParents.remove(widget);
        
        // Supprimer aussi l'association bouton -> sous-menu
        for (auto it = m_buttonToSubMenu.begin(); it != m_buttonToSubMenu.end();) {
            if (it.value() == widget) {
                it = m_buttonToSubMenu.erase(it);
            } else {
                ++it;
            }
        }
        
        widget->deleteLater();
    }
}

QBoxLayout* LaunchBar::createPerpendicularLayout(QBoxLayout *parentLayout)
{
    if (qobject_cast<QHBoxLayout*>(parentLayout)) {
        return new QVBoxLayout();
    } else {
        return new QHBoxLayout();
    }
}

void LaunchBar::setPosition(Position pos)
{
    m_position = pos;
    
    // Fermer tous les sous-menus
    closeChildMenus(this);
    m_buttonToSubMenu.clear();
    
    delete m_layout;
    if (pos == Left || pos == Right) {
        m_layout = new QVBoxLayout(this);
    } else {
        m_layout = new QHBoxLayout(this);
    }
    m_layout->setContentsMargins(10, 10, 10, 10);
    m_layout->setSpacing(5);
    
    for (LaunchButton *btn : m_buttons) {
        m_layout->addWidget(btn);
    }
    
    updatePosition();
}

void LaunchBar::updatePosition()
{
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    
    adjustSize();
    
    int x = 0, y = 0;
    
    switch (m_position) {
        case Bottom:
            x = (screenGeometry.width() - width()) / 2;
            y = screenGeometry.height() - height() - 10;
            break;
        case Top:
            x = (screenGeometry.width() - width()) / 2;
            y = 10;
            break;
        case Left:
            x = 10;
            y = (screenGeometry.height() - height()) / 2;
            break;
        case Right:
            x = screenGeometry.width() - width() - 10;
            y = (screenGeometry.height() - height()) / 2;
            break;
    }
    
    move(x, y);
}

void LaunchBar::clearButtons()
{
    for (LaunchButton *btn : m_buttons) {
        m_layout->removeWidget(btn);
        btn->deleteLater();
    }
    m_buttons.clear();
}

void LaunchBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        // Fermer tous les sous-menus quand on commence à déplacer la barre
        closeChildMenus(this); // Fermer tous les menus de la barre principale
        m_buttonToSubMenu.clear(); // Nettoyer toutes les associations
        m_dragging = true;
        m_dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void LaunchBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void LaunchBar::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    QPainterPath path;
    path.addRoundedRect(rect(), 15, 15);
    
    QColor bgColor = m_backgroundColor;
    bgColor.setAlpha(m_opacity);
    painter.fillPath(path, bgColor);
    painter.setPen(QPen(QColor(80, 80, 80), 1));
    painter.drawPath(path);
    
    QWidget::paintEvent(event);
}

void LaunchBar::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void LaunchBar::dropEvent(QDropEvent *event)
{
    const QMimeData *mimeData = event->mimeData();
    
    if (mimeData->hasUrls()) {
        // Fermer tous les sous-menus
        closeChildMenus(this);
        m_buttonToSubMenu.clear();
        
        QList<QUrl> urls = mimeData->urls();
        for (const QUrl &url : urls) {
            QString filePath = url.toLocalFile();
            QFileInfo fileInfo(filePath);
            
            if (fileInfo.suffix() == "desktop") {
                addProgramFromDesktopFile(filePath);
            } else if (fileInfo.isExecutable() || fileInfo.isFile()) {
                addProgramFromExecutable(filePath);
            }
        }
        
        saveConfig();
        clearButtons();
        createButtons(m_items);
        adjustSize();
        updatePosition();
        
        event->acceptProposedAction();
    }
}

void LaunchBar::addProgramFromDesktopFile(const QString &desktopFilePath)
{
    QSettings desktopFile(desktopFilePath, QSettings::IniFormat);
    desktopFile.beginGroup("Desktop Entry");
    
    QString name = desktopFile.value("Name").toString();
    QString exec = desktopFile.value("Exec").toString();
    QString icon = desktopFile.value("Icon").toString();
    
    exec = exec.remove(QRegExp("%[fFuU]"));
    exec = exec.trimmed();
    
    desktopFile.endGroup();
    
    if (exec.isEmpty()) {
        qWarning() << "Impossible de trouver la commande dans" << desktopFilePath;
        return;
    }
    
    if (!icon.isEmpty() && !QFileInfo(icon).exists()) {
        QString iconPath = findIconForProgram(icon);
        if (!iconPath.isEmpty()) {
            icon = iconPath;
        }
    }
    
    QJsonObject newItem;
    newItem["program"] = exec;
    newItem["icon"] = icon.isEmpty() ? "/usr/share/icons/hicolor/48x48/apps/application-x-executable.png" : icon;
    newItem["label"] = name;
    
    m_items.append(newItem);
    
    qDebug() << "Programme ajouté:" << name << "(" << exec << ")";
}

void LaunchBar::addProgramFromExecutable(const QString &executablePath)
{
    QFileInfo fileInfo(executablePath);
    QString programName = fileInfo.fileName();
    
    QString iconPath;
    
    if (fileInfo.isExecutable()) {
        iconPath = findIconForProgram(programName);
        if (iconPath.isEmpty()) {
            iconPath = "/usr/share/icons/hicolor/48x48/apps/application-x-executable.png";
        }
    } else {
        iconPath = getIconForMimeType(executablePath);
    }
    
    QJsonObject newItem;
    newItem["program"] = executablePath;
    newItem["icon"] = iconPath;
    newItem["label"] = programName;
    
    m_items.append(newItem);
    
    qDebug() << "Fichier ajouté:" << programName;
}

QString LaunchBar::findIconForProgram(const QString &programName)
{
    QStringList extensions = {".png", ".svg", ".xpm"};
    
    for (const QString &path : m_iconPaths) {
        for (const QString &ext : extensions) {
            QString fullPath = path + programName + ext;
            if (QFileInfo::exists(fullPath)) {
                return fullPath;
            }
        }
    }
    
    return QString();
}

QString LaunchBar::getIconForMimeType(const QString &filePath)
{
#if QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
    QMimeDatabase mimeDatabase;
    QMimeType mimeType = mimeDatabase.mimeTypeForFile(filePath);
    
    QString iconName = mimeType.iconName();
    
    QStringList iconPaths = QStringList() << "/usr/share/icons/hicolor/48x48/mimetypes/"<< "/usr/share/icons/hicolor/64x64/mimetypes/"<< "/usr/share/icons/gnome/48x48/mimetypes/" <<"/usr/share/pixmaps/" ;
    
    QStringList extensions = {".png", ".svg", ".xpm"};
    
    for (const QString &path : iconPaths) {
        for (const QString &ext : extensions) {
            QString fullPath = path + iconName + ext;
            if (QFileInfo::exists(fullPath)) {
                return fullPath;
            }
        }
    }
#endif
    
    return "/usr/share/icons/hicolor/48x48/mimetypes/text-x-generic.png";
}

bool LaunchBar::isHorizontalLayout() const
{
    return (m_position == Top || m_position == Bottom);
}

void LaunchBar::removeButton(LaunchButton *button)
{
    int index = button->buttonIndex();
    if (index >= 0 && index < m_items.size()) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, 
            "Confirmer la suppression",
            "Voulez-vous vraiment supprimer ce programme ?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            QJsonArray newItems;
            for (int i = 0; i < m_items.size(); ++i) {
                if (i != index) {
                    newItems.append(m_items[i]);
                }
            }
            m_items = newItems;
            
            saveConfig();
            QTimer::singleShot(0, this, [this]() {
                clearButtons();
                createButtons(m_items);
                adjustSize();
                updatePosition();
            });
        }
    }
}

void LaunchBar::editButton(LaunchButton *button)
{
    int index = button->buttonIndex();
    if (index < 0 || index >= m_items.size()) return;
    
    QDialog dialog(this);
    dialog.setWindowTitle("Propriétés du programme");
    dialog.setModal(true);
    dialog.resize(500, 200);
    
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *programEdit = new QLineEdit(button->program());
    formLayout->addRow("Programme:", programEdit);
    
    QComboBox *iconCombo = new QComboBox();
    iconCombo->setEditable(true);
    iconCombo->setInsertPolicy(QComboBox::NoInsert);
    iconCombo->addItem(button->iconPath());
    
    QStringList availableIcons = findAvailableIcons();
    int maxIcons = qMin(100, availableIcons.size());
    for (int i = 0; i < maxIcons; ++i) {
        const QString &iconPath = availableIcons[i];
        if (iconPath != button->iconPath()) {
            QFileInfo info(iconPath);
            //iconCombo->addItem(QIcon(iconPath), info.fileName(), iconPath);
            iconCombo->addItem(QIcon(iconPath), iconPath, iconPath);
        }
    }
    iconCombo->setCurrentText(button->iconPath());
    
    QPushButton *browseButton = new QPushButton("Parcourir...");
    
    QHBoxLayout *iconRow = new QHBoxLayout();
    iconRow->addWidget(iconCombo, 1);
    iconRow->addWidget(browseButton);
    formLayout->addRow("Icône:", iconRow);
    
    connect(browseButton, &QPushButton::clicked, [&iconCombo, &dialog]() {
        QString iconPath = QFileDialog::getOpenFileName(
            &dialog,
            "Sélectionner une icône",
            "/usr/share/icons",
            "Images (*.png *.svg *.xpm *.jpg)"
        );
        if (!iconPath.isEmpty()) {
            iconCombo->setCurrentText(iconPath);
        }
    });
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject item = m_items[index].toObject();
        item["program"] = programEdit->text();
        item["icon"] = iconCombo->currentText();
        m_items[index] = item;
        
        saveConfig();
        QTimer::singleShot(0, this, [this]() {
            clearButtons();
            createButtons(m_items);
            adjustSize();
            updatePosition();
        });
    }
}

QJsonObject* LaunchBar::findJsonObject(const QList<int> &path)
{
    if (path.isEmpty()) return nullptr;
    
    // Commencer au niveau racine
    if (path[0] >= m_items.size()) return nullptr;
    
    QJsonObject current = m_items[path[0]].toObject();
    
    // Naviguer dans la hiérarchie
    for (int i = 1; i < path.size(); ++i) {
        if (!current.contains("category")) return nullptr;
        
        QJsonArray category = current["category"].toArray();
        if (path[i] >= category.size()) return nullptr;
        
        current = category[path[i]].toObject();
    }
    
    // Retourner un pointeur vers l'objet dans m_items
    // Note: On doit modifier m_items directement
    static QJsonObject tempObject;
    tempObject = current;
    return &tempObject;
}

bool LaunchBar::removeJsonObject(const QList<int> &path)
{
    if (path.isEmpty()) return false;
    
    if (path.size() == 1) {
        // Suppression au niveau racine
        if (path[0] >= m_items.size()) return false;
        
        QJsonArray newItems;
        for (int i = 0; i < m_items.size(); ++i) {
            if (i != path[0]) {
                newItems.append(m_items[i]);
            }
        }
        m_items = newItems;
        return true;
    }
    
    // Suppression dans une catégorie
    if (path[0] >= m_items.size()) return false;
    
    QJsonObject root = m_items[path[0]].toObject();
    QJsonObject *current = &root;
    
    // Naviguer jusqu'à l'avant-dernier niveau
    for (int i = 1; i < path.size() - 1; ++i) {
        if (!current->contains("category")) return false;
        
        QJsonArray category = (*current)["category"].toArray();
        if (path[i] >= category.size()) return false;
        
        QJsonObject temp = category[path[i]].toObject();
        *current = temp;
    }
    
    // Supprimer l'élément au dernier niveau
    if (!current->contains("category")) return false;
    
    QJsonArray category = (*current)["category"].toArray();
    int lastIndex = path.last();
    
    if (lastIndex >= category.size()) return false;
    
    QJsonArray newCategory;
    for (int i = 0; i < category.size(); ++i) {
        if (i != lastIndex) {
            newCategory.append(category[i]);
        }
    }
    
    // Remonter la modification
    // Cette approche est complexe, utilisons une approche récursive
    m_items = removeFromArray(m_items, path, 0);
    
    return true;
}

QJsonArray LaunchBar::removeFromArray(const QJsonArray &array, const QList<int> &path, int depth)
{
    if (depth >= path.size()) return array;
    
    QJsonArray result;
    for (int i = 0; i < array.size(); ++i) {
        if (i == path[depth]) {
            if (depth == path.size() - 1) {
                // C'est l'élément à supprimer, on ne l'ajoute pas
                continue;
            } else {
                // On doit descendre plus profond
                QJsonObject obj = array[i].toObject();
                if (obj.contains("category")) {
                    QJsonArray subArray = obj["category"].toArray();
                    obj["category"] = removeFromArray(subArray, path, depth + 1);
                }
                result.append(obj);
            }
        } else {
            result.append(array[i]);
        }
    }
    return result;
}

QJsonArray LaunchBar::updateInArray(const QJsonArray &array, const QList<int> &path, int depth, const QString &program, const QString &icon)
{
    if (depth >= path.size()) return array;
    
    QJsonArray result;
    for (int i = 0; i < array.size(); ++i) {
        if (i == path[depth]) {
            if (depth == path.size() - 1) {
                // C'est l'élément à mettre à jour
                QJsonObject obj = array[i].toObject();
                obj["program"] = program;
                obj["icon"] = icon;
                result.append(obj);
            } else {
                // On doit descendre plus profond
                QJsonObject obj = array[i].toObject();
                if (obj.contains("category")) {
                    QJsonArray subArray = obj["category"].toArray();
                    obj["category"] = updateInArray(subArray, path, depth + 1, program, icon);
                }
                result.append(obj);
            }
        } else {
            result.append(array[i]);
        }
    }
    return result;
}

void LaunchBar::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    
    QAction *addCategoryAction = menu.addAction("Ajouter une catégorie");
    menu.addSeparator();
    
    QAction *preferencesAction = menu.addAction("Préférences");
    menu.addSeparator();
    
    QMenu *positionMenu = menu.addMenu("Position");
    QAction *topAction = positionMenu->addAction("En haut");
    QAction *bottomAction = positionMenu->addAction("En bas");
    QAction *leftAction = positionMenu->addAction("À gauche");
    QAction *rightAction = positionMenu->addAction("À droite");
    
    switch (m_position) {
        case Top: topAction->setCheckable(true); topAction->setChecked(true); break;
        case Bottom: bottomAction->setCheckable(true); bottomAction->setChecked(true); break;
        case Left: leftAction->setCheckable(true); leftAction->setChecked(true); break;
        case Right: rightAction->setCheckable(true); rightAction->setChecked(true); break;
    }
    
    menu.addSeparator();
    QAction *quitAction = menu.addAction("Quitter");
    
    QAction *selectedAction = menu.exec(event->globalPos());
    
    if (selectedAction == addCategoryAction) {
        addCategory();
    } else if (selectedAction == preferencesAction) {
        showPreferences();
    } else if (selectedAction == topAction) {
        setPosition(Top);
        saveConfig();
    } else if (selectedAction == bottomAction) {
        setPosition(Bottom);
        saveConfig();
    } else if (selectedAction == leftAction) {
        setPosition(Left);
        saveConfig();
    } else if (selectedAction == rightAction) {
        setPosition(Right);
        saveConfig();
    } else if (selectedAction == quitAction) {
        QApplication::quit();
    }
}

void LaunchBar::addCategory()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Nouvelle catégorie");
    dialog.setModal(true);
    
    QFormLayout *formLayout = new QFormLayout();
    
    QLineEdit *nameEdit = new QLineEdit();
    formLayout->addRow("Nom:", nameEdit);
    
    QHBoxLayout *iconLayout = new QHBoxLayout();
    QLineEdit *iconEdit = new QLineEdit("/usr/share/icons/hicolor/48x48/apps/folder.png");
    QPushButton *browseButton = new QPushButton("Parcourir...");
    iconLayout->addWidget(iconEdit);
    iconLayout->addWidget(browseButton);
    formLayout->addRow("Icône:", iconLayout);
    
    connect(browseButton, &QPushButton::clicked, [&]() {
        QString iconPath = QFileDialog::getOpenFileName(
            &dialog,
            "Sélectionner une icône",
            "/usr/share/icons",
            "Images (*.png *.svg *.xpm *.jpg)"
        );
        if (!iconPath.isEmpty()) {
            iconEdit->setText(iconPath);
        }
    });
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted && !nameEdit->text().isEmpty()) {
        QJsonObject newCategory;
        newCategory["label"] = nameEdit->text();
        newCategory["icon"] = iconEdit->text();
        newCategory["category"] = QJsonArray();
        
        m_items.append(newCategory);
        
        saveConfig();
        QTimer::singleShot(0, this, [this]() {
            clearButtons();
            createButtons(m_items);
            adjustSize();
            updatePosition();
        });
    }
}

void LaunchBar::handleDropOnCategory(LaunchButton *category, const QString &filePath)
{
    int categoryIndex = category->buttonIndex();
    if (categoryIndex < 0 || categoryIndex >= m_items.size()) return;
    
    QFileInfo fileInfo(filePath);
    QString iconPath;
    QString program = filePath;
    QString label = fileInfo.fileName();
    
    if (fileInfo.suffix() == "desktop") {
        QSettings desktopFile(filePath, QSettings::IniFormat);
        desktopFile.beginGroup("Desktop Entry");
        label = desktopFile.value("Name").toString();
        program = desktopFile.value("Exec").toString();
        program = program.remove(QRegExp("%[fFuU]")).trimmed();
        iconPath = desktopFile.value("Icon").toString();
        desktopFile.endGroup();
        
        if (!QFileInfo(iconPath).exists()) {
            QString foundIcon = findIconForProgram(iconPath);
            if (!foundIcon.isEmpty()) iconPath = foundIcon;
        }
    } else if (fileInfo.isExecutable()) {
        iconPath = findIconForProgram(fileInfo.fileName());
    } else {
        iconPath = getIconForMimeType(filePath);
    }
    
    if (iconPath.isEmpty()) {
        iconPath = "/usr/share/icons/hicolor/48x48/apps/application-x-executable.png";
    }
    
    QJsonObject categoryItem = m_items[categoryIndex].toObject();
    QJsonArray categoryChildren = categoryItem["category"].toArray();
    
    QJsonObject newItem;
    newItem["icon"] = iconPath;
    newItem["program"] = program;
    newItem["label"] = label;
    
    categoryChildren.append(newItem);
    categoryItem["category"] = categoryChildren;
    m_items[categoryIndex] = categoryItem;
    
    saveConfig();
    qDebug() << "Ajouté à la catégorie:" << label;
}

QStringList LaunchBar::findAvailableIcons()
{
    QStringList icons;
    
    for (const QString &path : m_iconPaths) {
        QDir dir(path);
        if (dir.exists()) {
            QStringList filters;
            filters << "*.png" << "*.svg" << "*.xpm";
            QStringList files = dir.entryList(filters, QDir::Files);
            for (const QString &file : files) {
                icons.append(path + file);
            }
        }
    }
    
    icons.sort();
    icons.removeDuplicates();
    return icons;
}

void LaunchBar::showPreferences()
{
    QDialog dialog(this);
    dialog.setWindowTitle("Préférences");
    dialog.setModal(true);
    dialog.resize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
    
    QTabWidget *tabs = new QTabWidget();
    
    QWidget *appearanceTab = new QWidget();
    QFormLayout *appearanceLayout = new QFormLayout(appearanceTab);
    
    QSlider *opacitySlider = new QSlider(Qt::Horizontal);
    opacitySlider->setRange(50, 255);
    opacitySlider->setValue(m_opacity);
    QLabel *opacityLabel = new QLabel(QString::number(m_opacity));
    connect(opacitySlider, &QSlider::valueChanged, [opacityLabel](int value) {
        opacityLabel->setText(QString::number(value));
    });
    
    QHBoxLayout *opacityLayout = new QHBoxLayout();
    opacityLayout->addWidget(opacitySlider);
    opacityLayout->addWidget(opacityLabel);
    appearanceLayout->addRow("Opacité:", opacityLayout);
    
    QPushButton *colorButton = new QPushButton();
    colorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
        .arg(m_backgroundColor.red())
        .arg(m_backgroundColor.green())
        .arg(m_backgroundColor.blue()));
    colorButton->setFixedSize(100, 30);
    
    connect(colorButton, &QPushButton::clicked, [this, colorButton]() {
        QColor color = QColorDialog::getColor(m_backgroundColor, this, "Choisir la couleur");
        if (color.isValid()) {
            m_backgroundColor = color;
            colorButton->setStyleSheet(QString("background-color: rgb(%1, %2, %3);")
                .arg(color.red())
                .arg(color.green())
                .arg(color.blue()));
        }
    });
    
    appearanceLayout->addRow("Couleur de fond:", colorButton);
    
    // Taille des icônes
    QComboBox *iconSizeCombo = new QComboBox();
    iconSizeCombo->addItem("32x32", 32);
    iconSizeCombo->addItem("48x48", 48);
    iconSizeCombo->addItem("64x64", 64);
    iconSizeCombo->addItem("96x96", 96);
    iconSizeCombo->addItem("128x128", 128);
    
    // Sélectionner la taille actuelle
    int currentIndex = iconSizeCombo->findData(m_iconSize);
    if (currentIndex >= 0) {
        iconSizeCombo->setCurrentIndex(currentIndex);
    }
    appearanceLayout->addRow("Taille des icônes:", iconSizeCombo);
    
    tabs->addTab(appearanceTab, "Apparence");
    
    // Onglet Chemins d'icônes
    QWidget *pathsTab = new QWidget();
    QVBoxLayout *pathsLayout = new QVBoxLayout(pathsTab);
    
    pathsLayout->addWidget(new QLabel("Chemins de recherche des icônes:"));
    
    QListWidget *pathsList = new QListWidget();
    for (const QString &path : m_iconPaths) {
        pathsList->addItem(path);
    }
    pathsLayout->addWidget(pathsList);
    
    QHBoxLayout *pathsButtons = new QHBoxLayout();
    QPushButton *addPathBtn = new QPushButton("Ajouter");
    QPushButton *removePathBtn = new QPushButton("Supprimer");
    pathsButtons->addWidget(addPathBtn);
    pathsButtons->addWidget(removePathBtn);
    pathsButtons->addStretch();
    pathsLayout->addLayout(pathsButtons);
    
    connect(addPathBtn, &QPushButton::clicked, [pathsList, this]() {
        QString path = QFileDialog::getExistingDirectory(
            this,
            "Sélectionner un dossier d'icônes",
            "/usr/share/icons"
        );
        if (!path.isEmpty()) {
            if (!path.endsWith("/")) path += "/";
            pathsList->addItem(path);
        }
    });
    
    connect(removePathBtn, &QPushButton::clicked, [pathsList]() {
        QListWidgetItem *item = pathsList->currentItem();
        if (item) {
            delete item;
        }
    });
    
    tabs->addTab(pathsTab, "Chemins d'icônes");
    
    // Onglet JSON
    QWidget *jsonTab = new QWidget();
    QVBoxLayout *jsonLayout = new QVBoxLayout(jsonTab);
    
    QTextEdit *jsonEdit = new QTextEdit();
    QJsonDocument doc(QJsonObject{
        {"position", m_position == Top ? "top" : m_position == Left ? "left" : m_position == Right ? "right" : "bottom"},
        {"items", m_items},
        {"opacity", m_opacity},
        {"backgroundColor", QJsonObject{{"r", m_backgroundColor.red()}, {"g", m_backgroundColor.green()}, {"b", m_backgroundColor.blue()}}}
    });
    jsonEdit->setPlainText(doc.toJson(QJsonDocument::Indented));
    jsonEdit->setFont(QFont("Monospace", 9));
    
    jsonLayout->addWidget(new QLabel("Éditer le fichier de configuration JSON:"));
    jsonLayout->addWidget(jsonEdit);
    tabs->addTab(jsonTab, "JSON");
    
    mainLayout->addWidget(tabs);
    
    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel
    );
    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    mainLayout->addWidget(buttonBox);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_opacity = opacitySlider->value();
        
        QString jsonText = jsonEdit->toPlainText();
        QJsonDocument newDoc = QJsonDocument::fromJson(jsonText.toUtf8());
        
        if (!newDoc.isNull() && newDoc.isObject()) {
            QJsonObject root = newDoc.object();
            if (root.contains("items") && root["items"].isArray()) {
                m_items = root["items"].toArray();
            }
        }
        
        saveConfig();
        QTimer::singleShot(0, this, [this]() {
            clearButtons();
            createButtons(m_items);
            adjustSize();
            update();  // Juste rafraîchir l'affichage, sans bouger la fenêtre
        });
    }
}
