#include "AutoHeader.h"
#include "ui_AutoHeader.h"
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QString>
#include <QChar>
#include <QFileDialog>
#include <QMessageBox>

AutoHeader::AutoHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoHeader),
    is_source_file_selected(false),
    is_destination_path_selected(false),
    is_clang_file_selected(false),
    is_add_comments_check_box_checked_(true),
    is_apply_clang_check_box_checked_(true),
    is_auto_brief_check_box_checked_(true)
{
    ui->setupUi(this);
    this->setFixedWidth(600);

    ui->add_comments_check_box_->setChecked(true);
    ui->apply_clang_check_box_->setChecked(true);
    ui->auto_brief_check_box_->setChecked(true);
    connect(ui->SourceBrowseButton, SIGNAL(clicked()), this, SLOT(OnSourceBrowseButtonClicked()));
    connect(ui->DestinationDirectoryButton, SIGNAL(clicked()), this, SLOT(OnDestinationBrowseButtonClicked()));
}

AutoHeader::~AutoHeader()
{
    delete ui;
}

void AutoHeader::read_source_file(const QString& file_name, QString output_path)
{
    QFile file(file_name);

    QStringList file_name_list = file_name.split("/");
    QString file_name_string = file_name_list.at(file_name_list.size() - 1);
    QString output_file_name = output_path.append("/").append(file_name_string);
    QFile output_file(output_file_name);

    qDebug() << "output_file_name " << output_file_name;

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "error opening file: " << file.error();
        return;
    }

    if(!output_file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "error opening output file: " << file.error();
        return;
    }

    QString previous_line;

    QTextStream in(&file);
    QString class_name;

    int line_number = 1;
    while (!in.atEnd()) {
        QString line = in.readLine();

        qDebug() << "first line: " << line;

        QTextStream out(&output_file);

        if(!line.trimmed().isEmpty())
        {
            if(line_number == 1 && !is_file_header_present(line))
            {
                out<< "/*! @file "<<file_name_string.trimmed()<<"\n";
                add_file_brief(out, is_auto_brief_check_box_checked_, file_name_string);
                out<< " */"<<"\n";
                out<<"\n";
            }

            if(line.contains(":"))
            {
                if(line.contains("public:") || line.contains("protected:")  ||
                        line.contains("private:") || line.contains("signals:") ||
                        line.contains("public slots:") || line.contains("protected slots:") ||
                        line.contains("private slots:"))
                {
                    QString input_line = line;
                    access_specifier = input_line.remove(":").trimmed();
                }
            }
            if(is_class(line, class_name))
            {
                if(!previous_line.contains("*/"))
                {
                    out << "\n";
                    out<< "/*! @"<<line.trimmed()<<"\n";
                    add_class_brief(out, is_auto_brief_check_box_checked_, class_name);
                    out<< " */"<<"\n";
                    out << line << "\n";
                }
                else {
                    out << line << "\n";
                }
            }
            else if(is_function(line))
            {
                if(!previous_line.contains("*/") && !line.contains(class_name))
                {
                    out << "\n";
                    out<< "    /*! @fn "<<line.trimmed()<<"\n";
                    add_funtion_brief(out, is_auto_brief_check_box_checked_, get_function_name(line.trimmed()));
                    add_parameter(line.trimmed(), out);
                    out<< "     * @return "<<get_return_type(line.trimmed())<<"\n";
                    out<< "     */"<<"\n";
                    out << line << "\n";
                }
                else {
                    out << line << "\n";
                }
            }
            else {
                out << line << "\n";

            }
            line_number++;
        }
        else {
            out << line << "\n";
        }
        previous_line = line;
    }


    file.close();
}

bool AutoHeader::is_function(const QString &line)
{
    if(line.contains("(") && line.contains(")") && !line.contains("/*!") && !line.contains("return "))
    {
        return true;
    }
    return false;
}

bool AutoHeader::is_class(QString line, QString& class_name)
{
    qDebug()<<" is_class line = "<<line;
    if(line.startsWith("class ") && !line.contains(";"))
    {
        class_name = line.remove("class ").remove("final").trimmed();
        if(class_name.contains(":"))
        {
            QStringList class_list = class_name.split(":");

            class_name = class_list.at(0).trimmed();
        }
        return true;
    }

    return false;
}

bool AutoHeader::is_file_header_present(const QString &line)
{
    if(line.startsWith("/*"))
    {
        return true;
    }
    return false;
}

QString AutoHeader::get_return_type(const QString &line)
{
    QStringList return_type_list = line.trimmed().split("(");
    QString final_return_string = return_type_list.at(0);


    QStringList space_list = final_return_string.trimmed().split(" ");
    final_return_string.clear();
    for(int j = 0; j < space_list.size(); j++)
    {
        if(j < (space_list.size() - 1))
        {
            final_return_string.append(space_list.at(j));
            final_return_string.append(" ");
        }
    }

    return final_return_string.trimmed();
}

void AutoHeader::add_parameter(const QString &line, QTextStream& out)
{
    QStringList multiple_parameter_list = line.split("(");
    QString function_name_with_return = multiple_parameter_list.at(0);
    QStringList function_name_list = function_name_with_return.split(" ");
    QString function_name = function_name_list.at(function_name_list.size() - 1);


    QString final_parameter = multiple_parameter_list.at(1);
    multiple_parameter_list.clear();
    multiple_parameter_list = final_parameter.split(")");

    final_parameter = multiple_parameter_list.at(0);

    if(final_parameter.isEmpty())
    {
        return;
    }
    multiple_parameter_list.clear();
    multiple_parameter_list = final_parameter.split(",");
    final_parameter.clear();

    for(int i = 0; i < multiple_parameter_list.size(); i++)
    {
        QString parameter = multiple_parameter_list.at(i);
        QStringList parameter_type_list = parameter.split(" ");

        if(!parameter.contains("="))
        {
            final_parameter = parameter_type_list.at(parameter_type_list.size() - 1);
            final_parameter.remove("*").remove("&");
            QString input_output = get_input_output_parameter(parameter, function_name);
            out<< "     * @param "<<input_output<< final_parameter<<"\n";
        }
        else {
            QStringList equal_string_list = parameter.split("=");
            QString before_equal = equal_string_list.at(0);
            before_equal = before_equal.trimmed();
            QStringList parameter_type_list_2 = before_equal.split(" ");
            final_parameter = parameter_type_list_2.at(parameter_type_list_2.size() - 1);
            final_parameter.remove("*").remove("&");
            QString input_output = get_input_output_parameter(parameter, function_name);
            out<< "     * @param "<<input_output<< final_parameter<<"\n";
        }
    }
}

void AutoHeader::add_funtion_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& function_name)
{
    if(!is_auto_brief_check_box_checked)
    {
        if(access_specifier.contains("slots"))
        {
            QString slot_type = access_specifier;
            slot_type.remove("slots");
            out<< "     * @brief This "<<slot_type<<" slot is responsible to "<<"\n";
        }
        else if(access_specifier.contains("public") || access_specifier.contains("protected") || access_specifier.contains("private"))
        {
            out<< "     * @brief This "<<access_specifier<<" function is responsible to "<<"\n";
        }

        else if(access_specifier.contains("signals"))
        {
            out<< "     * @brief This signal is responsible to "<<"\n";
        }
        else
        {
            out<< "     * @brief "<<"\n";
        }
    }
    else
    {

        QString function_brief = get_function_brief_string(function_name);

        if(access_specifier.contains("slots"))
        {
            QString slot_type = access_specifier;
            slot_type.remove("slots");
            out<< "     * @brief This "<<slot_type<<" slot is responsible to "<<function_brief<<"\n";
        }
        else if(access_specifier.contains("public") || access_specifier.contains("protected") || access_specifier.contains("private"))
        {
            out<< "     * @brief This "<<access_specifier<<" function is responsible to "<<function_brief<<"\n";
        }
        else if(access_specifier.contains("signals"))
        {
            out<< "     * @brief This signal is responsible to "<<function_brief<<"\n";
        }
        else
        {
            out<< "     * @brief "<<"\n";
        }
    }

}

void AutoHeader::add_class_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& class_name)
{
    if(!is_auto_brief_check_box_checked)
    {
        out<< " *  @brief This class is responsible to "<<"\n";
    }
    else
    {
        QString class_brief = get_class_brief_string(class_name);
        out<< " *  @brief This class is responsible to "<<class_brief<<"\n";
    }
}

void AutoHeader::add_file_brief(QTextStream& out, bool is_auto_brief_check_box_checked, const QString& file_name)
{
    if(!is_auto_brief_check_box_checked)
    {
        out<< " *  @brief This file is responsible to "<<"\n";
    }
    else
    {
        QString file_brief = get_class_brief_string(file_name);
        out<< " *  @brief This file is responsible to "<<file_brief<<"\n";
    }
}

QString AutoHeader::get_input_output_parameter(const QString& line, const QString& function_name)
{
    if(line.contains("const"))
    {
        return "[in] ";
    }
    else if(!line.contains("&") && !line.contains("*"))
    {
        return "[in] ";
    }
    else
    {
        if(function_name.contains("set") || function_name.contains("Set"))
        {
            return "[in] ";
        }
        return "[out] ";
    }
}

QString AutoHeader::get_function_name(const QString &line)
{
    QStringList multiple_parameter_list = line.split("(");
    QString function_name_with_return = multiple_parameter_list.at(0);
    QStringList function_name_list = function_name_with_return.split(" ");
    QString function_name = function_name_list.at(function_name_list.size() - 1);
    return function_name;
}

QString AutoHeader::get_function_brief_string(const QString& function_name)
{
    QString final_brief;

    for (int index = 0; index < function_name.size(); ++index)
    {
        QString string_data;
        if(function_name.at(index).isUpper())
        {
            string_data.append(" ");
        }
        string_data.append(function_name.at(index).toLower());

        if(string_data == "_")
        {
            string_data = " ";
        }

        final_brief.append(string_data);
    }

    if(final_brief.contains("slot") || final_brief.contains("Slot"))
    {
        final_brief.remove("slot");
        final_brief.remove("Slot");
    }

    final_brief.replace("init", "initialize");
    final_brief.append(".");

    return final_brief;
}

QString AutoHeader::get_class_brief_string(const QString& class_name)
{
    QString final_brief;

    for (int index = 0; index < class_name.size(); ++index)
    {
        QString string_data;
        if(class_name.at(index).isUpper())
        {
            string_data.append(" ");
        }
        string_data.append(class_name.at(index).toLower());

        if(string_data == "_")
        {
            string_data = " ";
        }

        final_brief.append(string_data);
    }

    final_brief.replace("controller", "control");
    final_brief.replace(".h", "");
    final_brief.replace(".cpp", "");
    final_brief.append(".");

    return final_brief;
}

void AutoHeader::OnSourceBrowseButtonClicked()
{
//    source_file_name = QFileDialog::getOpenFileName(this, tr("Select Source File"),
//                                                    "/home/ubuntu/",
//                                                    tr("Images (*.cpp *.h)"));

    source_file_directory = QFileDialog::getExistingDirectory(this, tr("Select Source File"),
                                                    "/home/ubuntu/");

    if(!source_file_directory.isEmpty())
    {
        is_source_file_selected = true;
        ui->SourceDirectoryLabel->setText("Source Directory: "+source_file_directory);
    }
}

void AutoHeader::OnDestinationBrowseButtonClicked()
{
    destination_dir_name = QFileDialog::getExistingDirectory(this);

    if(!destination_dir_name.isEmpty())
    {
        ui->DestinationDirectoryLabel->setText("Destination Directory: "+destination_dir_name);
        is_destination_path_selected = true;
    }
}

void AutoHeader::OnAddCommetsButtonClicked()
{
    if(!is_source_file_selected)
    {
        QMessageBox msgBox;
        msgBox.setText("Kindly Select Source Files Path To Add Comments.");
        msgBox.exec();
    }
    else if(!is_destination_path_selected)
    {
        QMessageBox msgBox;
        msgBox.setText("Kindly Select Destination Path.");
        msgBox.exec();
    }
    else {
        QDir directory(source_file_directory);
        QStringList images = directory.entryList(QStringList() << "*.h" << "*.cpp",QDir::Files);
        foreach(QString filename, images) {
            QString file_path = source_file_directory;
            QString file_name_with_path = file_path.append("/").append(filename);
            read_source_file(file_name_with_path, destination_dir_name);
            ui->resultlistWidget->addItem("Comments Added for File:   " + file_name_with_path);
        }
    }
}

void AutoHeader::OnApplyCLangButtonClicked()
{
    if(!is_clang_file_selected)
    {
        QMessageBox msgBox;
        msgBox.setText("Kindly select .clanf-format file.");
        msgBox.exec();
    }
    else
    {
        QString c_lang_system_command = "find ";
        c_lang_system_command.append(source_file_directory).append(" -iname '*.h' -o -iname '*.cpp' | xargs clang-format-6.0 -i -style=file");

        QString copy_file_command;

        copy_file_command.append("cp ").append(clang_format_file_name).append(" ").append(source_file_directory);
        system(copy_file_command.toStdString().c_str());
        QString remove_file_command;
        remove_file_command.append("rm ").append(source_file_directory).append("/.clang-format");

        system(c_lang_system_command.toStdString().c_str());
        system(remove_file_command.toStdString().c_str());
    }
}

void AutoHeader::on_clangBrowswePushButton_released()
{
    clang_format_file_name = QFileDialog::getOpenFileName(this, tr("Select .clang-format File"),
                                                    "/home/ubuntu/",
                                                    tr("Images (.clang-format)"));
    if(!clang_format_file_name.isEmpty())
    {
        is_clang_file_selected = true;
    }
}

void AutoHeader::on_add_comments_check_box__stateChanged(int arg1)
{
    qDebug()<<"on_add_comments_check_box__stateChanged = "<<arg1;

    if(arg1 == 2)
    {
        is_add_comments_check_box_checked_ = true;
    }
    else
    {
        is_add_comments_check_box_checked_ = false;
    }
}

void AutoHeader::on_apply_clang_check_box__stateChanged(int arg1)
{
    qDebug()<<"on_apply_clang_check_box__stateChanged = "<<arg1;

    if(arg1 == 2)
    {
        is_apply_clang_check_box_checked_ = true;
    }
    else
    {
        is_apply_clang_check_box_checked_ = false;
    }
}

void AutoHeader::on_auto_brief_check_box__stateChanged(int arg1)
{
    qDebug()<<"on_auto_brief_check_box__stateChanged = "<<arg1;

    if(arg1 == 2)
    {
        is_auto_brief_check_box_checked_ = true;
    }
    else
    {
        is_auto_brief_check_box_checked_ = false;
    }
}

void AutoHeader::on_start_button__released()
{
    if(is_add_comments_check_box_checked_ && is_apply_clang_check_box_checked_)
    {
        if(is_source_file_selected && is_destination_path_selected && is_clang_file_selected)
        {
            OnApplyCLangButtonClicked();
            OnAddCommetsButtonClicked();
        }
        else
        {
            if(!is_source_file_selected && is_destination_path_selected && is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select source files path to add comments.");
                msgBox.exec();
            }
            else if(!is_source_file_selected && !is_destination_path_selected && is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select source files path and destination path.");
                msgBox.exec();
            }
            else if(!is_source_file_selected && !is_destination_path_selected && !is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select source files path, destination path and .clang-format file.");
                msgBox.exec();
            }
            else if(is_source_file_selected && !is_destination_path_selected && !is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select destination path and .clang-format file.");
                msgBox.exec();
            }
            else if(is_source_file_selected && !is_destination_path_selected && is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly destination path.");
                msgBox.exec();
            }
            else if(is_source_file_selected && is_destination_path_selected && !is_clang_file_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select .clanf-format file.");
                msgBox.exec();
            }
        }
    }
    else if(!is_add_comments_check_box_checked_ && is_apply_clang_check_box_checked_)
    {
        if(is_clang_file_selected)
        {
            OnApplyCLangButtonClicked();
        }
        else
        {
            QMessageBox msgBox;
            msgBox.setText("Kindly select .clanf-format file.");
            msgBox.exec();
        }
    }
    else if(is_add_comments_check_box_checked_ && !is_apply_clang_check_box_checked_)
    {
        if(is_source_file_selected)
        {
            OnAddCommetsButtonClicked();
        }
        else
        {
            if(!is_source_file_selected && is_destination_path_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select source files path to add comments.");
                msgBox.exec();
            }
            else if(!is_source_file_selected && !is_destination_path_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select source files path and destination path.");
                msgBox.exec();
            }
            else if(is_source_file_selected && !is_destination_path_selected)
            {
                QMessageBox msgBox;
                msgBox.setText("Kindly select destination path.");
                msgBox.exec();
            }
        }
    }
}
