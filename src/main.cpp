#include "appcontroller.h"
#include "ui/file_table_widget_model.h"
#include "ui/folder_tree_widget_model.h"
#include "ui/window.h"

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

    ui::Window      window;
    ui::FolderTreeWidgetModel folderModel;
    ui::FileTableWidgetModel  fileModel;

    AppController controller(&folderModel, &fileModel, &window);
    controller.start();

    window.show();
    return app.exec();
}
