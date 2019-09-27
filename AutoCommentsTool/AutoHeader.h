#ifndef AUTOHEADER_H
#define AUTOHEADER_H

#include <QWidget>

namespace Ui {
class AutoHeader;
}

class AutoHeader : public QWidget
{
    Q_OBJECT

public:
    explicit AutoHeader(QWidget *parent = nullptr);
    ~AutoHeader();

private slots:

    void OnSourceBrowseButtonClicked();
    void OnDestinationBrowseButtonClicked();
    void OnAddCommetsButtonClicked();
    void OnApplyCLangButtonClicked();
    void OnApplyBothButtonClicked();

private:
    Ui::AutoHeader *ui;
    QString source_file_directory;
    QString destination_dir_name;
    bool is_source_file_selected;
    bool is_destination_path_selected;

    void read_source_file(const QString& file_name, QString output_file_name);
    bool is_function(const QString& line);
    bool is_class(QString line, QString &class_name);
    bool is_file_header_present(const QString& line);

    QString get_return_type(const QString& line);
    QString get_parameter(const QString &line);

};

#endif // AUTOHEADER_H
