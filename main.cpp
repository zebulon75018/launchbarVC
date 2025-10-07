#include <QApplication>
#include "launchbar.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    LaunchBar launchBar;
    launchBar.show();
    
    return app.exec();
}
