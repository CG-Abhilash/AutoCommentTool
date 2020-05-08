#include "AutoHeader.h"
#include "ui_AutoHeader.h"
#include <QFile>
#include <QDebug>
#include <iostream>
#include <QString>
#include <QFileDialog>
#include <QMessageBox>

AutoHeader::AutoHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutoHeader),
    is_source_file_selected(false),
    is_destination_path_selected(false)
{
    ui->setupUi(this);
    this->setFixedSize(600,500);

    connect(ui->SourceBrowseButton, SIGNAL(clicked()), this, SLOT(OnSourceBrowseButtonClicked()));
    connect(ui->DestinationDirectoryButton, SIGNAL(clicked()), this, SLOT(OnDestinationBrowseButtonClicked()));
    connect(ui->AddCommetsButton, SIGNAL(clicked()), this, SLOT(OnAddCommetsButtonClicked()));
    connect(ui->ApplyCLangButton, SIGNAL(clicked()), this, SLOT(OnApplyCLangButtonClicked()));
    connect(ui->ApplyBothButton, SIGNAL(clicked()), this, SLOT(OnApplyBothButtonClicked()));
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
                out<< " *  @brief This file is responsible to "<<"\n";
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
                    out<< " *  @brief This class is responsible to "<<"\n";
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
                    add_funtion_brief(out);
                    get_parameter(line.trimmed(), out);
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

void AutoHeader::get_parameter(const QString &line, QTextStream& out)
{
    QStringList multiple_parameter_list = line.split("(");
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
            QString input_output = get_input_output_parameter(parameter);
            out<< "     * @param "<<input_output<< final_parameter<<"\n";
        }
        else {
            QStringList equal_string_list = parameter.split("=");
            QString before_equal = equal_string_list.at(0);
            before_equal = before_equal.trimmed();
            QStringList parameter_type_list_2 = before_equal.split(" ");
            final_parameter = parameter_type_list_2.at(parameter_type_list_2.size() - 1);
            QString input_output = get_input_output_parameter(before_equal);
            out<< "     * @param "<<input_output<< final_parameter<<"\n";
        }
    }
}

void AutoHeader::add_funtion_brief(QTextStream& out)
{

    if(access_specifier.contains("public") || access_specifier.contains("protected") || access_specifier.contains("private"))
    {
        out<< "     * @brief This "<<access_specifier<<" function is responsible to "<<"\n";
    }
    else if(access_specifier.contains("slots"))
    {
        QString slot_type = access_specifier;
        slot_type.remove("slots");
        out<< "     * @brief This "<<slot_type<<" slot is responsible to "<<"\n";
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

QString AutoHeader::get_input_output_parameter(const QString& line)
{
    if(line.contains("const"))
    {
        return "[in] ";
    }
    if(!line.contains("&") && !line.contains("*"))
    {
        return "[in] ";
    }
    else
    {
        return "[out] ";
    }
}

void AutoHeader::OnSourceBrowseButtonClicked()
{
    is_source_file_selected = true;
//    source_file_name = QFileDialog::getOpenFileName(this, tr("Select Source File"),
//                                                    "/home/ubuntu/",
//                                                    tr("Images (*.cpp *.h)"));

    source_file_directory = QFileDialog::getExistingDirectory(this, tr("Select Source File"),
                                                    "/home/ubuntu/");
    ui->SourceDirectoryLabel->setText("Source Directory: "+source_file_directory);
}

void AutoHeader::OnDestinationBrowseButtonClicked()
{
    is_destination_path_selected = true;
    destination_dir_name = QFileDialog::getExistingDirectory(this);
    ui->DestinationDirectoryLabel->setText("Destination Directory: "+destination_dir_name);
}

void AutoHeader::OnAddCommetsButtonClicked()
{
    if(!is_source_file_selected)
    {
        QMessageBox msgBox;
        msgBox.setText("Kindly Select Source File To Add Comments.");
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
    QString c_lang_system_command = "find ";
    c_lang_system_command.append(destination_dir_name).append(" -iname '*.h' -o -iname '*.cpp' | xargs clang-format-6.0 -i -style=file");

    QStringList x_lang_file_path_list = source_file_directory.split("/");

    QString c_lang_file_path;

    for(QString path : x_lang_file_path_list)
    {
        if(!path.isEmpty())
        {
            c_lang_file_path.append("/").append(path);

            if(path == "Marilyn_EM1")
            {
                c_lang_file_path.append("/").append(".clang-format");
                break;
            }
        }

    }

    QString copy_file_command;
    copy_file_command.append("cp ").append(c_lang_file_path).append(" ").append(destination_dir_name);
    system(copy_file_command.toStdString().c_str());

    QString remove_file_command;
    remove_file_command.append("rm ").append(destination_dir_name).append("/.clang-format");
    system(c_lang_system_command.toStdString().c_str());
    system(remove_file_command.toStdString().c_str());
}

void AutoHeader::OnApplyBothButtonClicked()
{
    OnAddCommetsButtonClicked();
    OnApplyCLangButtonClicked();
}
