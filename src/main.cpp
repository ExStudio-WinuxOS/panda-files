#include "application.h"

int main(int argc, char *argv[])
{
    qunsetenv("QT_NO_GLIB");

    Application app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    app.init();
    return app.exec();
}
