#ifndef AUTOHEADER_H
#define AUTOHEADER_H

#include <QWidget>
#include <QTextStream>

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

    void on_clangBrowswePushButton_released();

    void on_add_comments_check_box__stateChanged(int arg1);

    void on_apply_clang_check_box__stateChanged(int arg1);

    void on_auto_brief_check_box__stateChanged(int arg1);

    void on_start_button__released();

private:
    Ui::AutoHeader *ui;
    QString source_file_directory;
    QString destination_dir_name;
    QString clang_format_file_name;
    bool is_source_file_selected;
    bool is_destination_path_selected;
    bool is_clang_file_selected;
    QString access_specifier;
    bool is_add_comments_check_box_checked_;
    bool is_apply_clang_check_box_checked_;
    bool is_auto_brief_check_box_checked_;

    void read_source_file(const QString& file_name, QString output_file_name);
    bool is_function(const QString& line);
    bool is_class(QString line, QString &class_name);
    bool is_file_header_present(const QString& line);

    QString get_return_type(const QString& line);
    void add_parameter(const QString& line, QTextStream& out);
    void add_funtion_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& function_name);
    void add_class_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& class_name);
    void add_file_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& file_name);

    QString get_input_output_parameter(const QString& line, const QString& function_name);
    QString get_function_name(const QString& line);
    QString get_function_brief_string(const QString& function_name);
    QString get_class_brief_string(const QString& class_name);

};

#endif // AUTOHEADER_H
