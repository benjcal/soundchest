#include "appcontroller.h"
#include "ui/filetablemodel.h"
#include "ui/foldertreemodel.h"
#include "ui/mainwindow.h"

#include <QApplication>

#include <oclero/qlementine/resources/ResourceInitialization.hpp>
#include <oclero/qlementine/style/QlementineStyle.hpp>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName(QStringLiteral("soundchest"));
    app.setApplicationDisplayName(QStringLiteral("Sound Chest"));
    app.setOrganizationName(QStringLiteral("soundchest"));

    oclero::qlementine::resources::initializeResources();

    auto *style = new oclero::qlementine::QlementineStyle;
    QApplication::setStyle(style);
    style->setThemeJsonPath(QStringLiteral(":/themes/gruvbox.json"));

    ui::MainWindow      window;
    ui::FolderTreeModel folderModel;
    ui::FileTableModel  fileModel;

    AppController controller(&folderModel, &fileModel, window.mainView());
    controller.start();

    window.show();
    return app.exec();
}
