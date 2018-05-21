#ifndef SV_NAVSTATEDITOR_H
#define SV_NAVSTATEDITOR_H

#include <QDialog>

namespace Ui {
class SvNavStatEditorDialog;
}

class SvNavStatEditor : public QDialog
{
  Q_OBJECT
  
public:
  explicit SvNavStatEditor(QWidget *parent = 0);
  ~SvNavStatEditor();
  
private:
  Ui::SvNavStatEditorDialog *ui;
};

#endif // SV_NAVSTATEDITOR_H
