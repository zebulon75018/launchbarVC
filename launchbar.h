#ifndef LAUNCHBAR_H
#define LAUNCHBAR_H

#include <QWidget>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QTimer>
#include <QMenu>
#include <QMap>

class LaunchBar;

#define CONFIG_FILE_JSON ".launchbar.json"

class LaunchButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal scale READ scale WRITE setScale)
    
public:
    LaunchButton(const QString &iconPath, const QString &program, 
                 const QJsonArray &children, const QString &label = QString(),
                 int iconSize = 64, QWidget *parent = nullptr);
    
    QString program() const { return m_program; }
    QJsonArray children() const { return m_children; }
    bool hasChildren() const { return !m_children.isEmpty(); }
    QString iconPath() const { return m_iconPath; }
    QString label() const { return m_label; }
    
    void setProgram(const QString &program) { m_program = program; }
    void setIconPath(const QString &iconPath);
    void setChildren(const QJsonArray &children) { m_children = children; }
    void setLabel(const QString &label) { m_label = label; update(); }
    
    qreal scale() const { return m_scale; }
    void setScale(qreal scale);
    
    int buttonIndex() const { return m_buttonIndex; }
    void setButtonIndex(int index) { m_buttonIndex = index; }
    
    QList<int> jsonPath() const { return m_jsonPath; }
    void setJsonPath(const QList<int> &path) { m_jsonPath = path; }
    
protected:
    void enterEvent(QEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    
signals:
    void removeRequested(LaunchButton *button);
    void editRequested(LaunchButton *button);
    void dropOnCategory(LaunchButton *category, const QString &filePath);
    
private:
    QString m_program;
    QString m_iconPath;
    QString m_label;
    QJsonArray m_children;
    qreal m_scale;
    QPropertyAnimation *m_animation;
    QPixmap m_iconPixmap;
    int m_buttonIndex;
    bool m_isHovered;
    int m_iconSize;
    QList<int> m_jsonPath; // Chemin dans la hiérarchie JSON
};

class LaunchBar : public QWidget
{
    Q_OBJECT
    
public:
    enum Position { Bottom, Top, Left, Right };
    
    LaunchBar(QWidget *parent = nullptr);
    bool loadConfig(const QString &configPath = CONFIG_FILE_JSON);
    void setPosition(Position pos);
    void saveConfig();
    
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    
private slots:
    void onButtonClicked();
    void removeButton(LaunchButton *button);
    void editButton(LaunchButton *button);
    void addCategory();
    void showPreferences();
    void handleDropOnCategory(LaunchButton *category, const QString &filePath);
    
private:
    void createButtons(const QJsonArray &items);
    void showSubButtons(const QJsonArray &children, LaunchButton *parent);
    void closeChildMenus(QWidget *parentWidget);
    void updatePosition();
    void clearButtons();
    void addProgramFromDesktopFile(const QString &desktopFilePath);
    void addProgramFromExecutable(const QString &executablePath);
    QString findIconForProgram(const QString &programName);
    QString getIconForMimeType(const QString &filePath);
    bool isHorizontalLayout() const;
    QStringList findAvailableIcons();
    QBoxLayout* createPerpendicularLayout(QBoxLayout *parentLayout);
    QJsonObject* findJsonObject(const QList<int> &path);
    bool removeJsonObject(const QList<int> &path);
    QJsonArray removeFromArray(const QJsonArray &array, const QList<int> &path, int depth);
    QJsonArray updateInArray(const QJsonArray &array, const QList<int> &path, int depth, const QString &program, const QString &icon);
    
    QBoxLayout *m_layout;
    Position m_position;
    QVector<LaunchButton*> m_buttons;
    QMap<QWidget*, QWidget*> m_subMenuParents; // Associe chaque sous-menu à son parent
    QMap<LaunchButton*, QWidget*> m_buttonToSubMenu; // Associe chaque bouton à son sous-menu
    QWidget *m_subButtonsWidget;
    QPoint m_dragPosition;
    bool m_dragging;
    QString m_configPath;
    QJsonArray m_items;
    int m_opacity;
    QColor m_backgroundColor;
    int m_iconSize;
    QStringList m_iconPaths;
};

#endif // LAUNCHBAR_H
