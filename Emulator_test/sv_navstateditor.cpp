#include "sv_navstateditor.h"
#include "ui_sv_navstateditor.h"

SvNavStatEditor::SvNavStatEditor(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::SvNavStatEditorDialog)
{
  ui->setupUi(this);
}

SvNavStatEditor::~SvNavStatEditor()
{
  delete ui;
}
