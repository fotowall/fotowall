/***************************************************************************
 *                                                                         *
 *   This file is part of the Fotowall project,                            *
 *       http://www.enricoros.com/opensource/fotowall                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   Original work                                                         *
 *      file             : richtexteditor.cpp                              *
 *      license          : LGPL v2.1 and GPL v3                            *
 *      copyright notice : follows below                                   *
 *                                                                         *
 ***************************************************************************/

/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the either Technology Preview License Agreement or the
** Beta Release License Agreement.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#define notImplemented() {qWarning("%s:%d: %s NOT Implemented!", __FILE__, __LINE__, __FUNCTION__);}
#include <QIcon>
#include <QFileDialog>
#include <QFontComboBox>
QIcon createIconSet(QString iconName)
{
    // remap for better names in the data dir
    if (iconName == "insertimage.png")
        iconName = "textinsertimage.png";
    return QIcon(":/data/" + iconName);
}


#include "richtexteditor_p.h"
#include "3rdparty/qtcolortriangle/QtColorTriangle"
 ///#include "htmlhighlighter_p.h"
 ///#include "iconselector_p.h"
 ///#include "ui_addlinkdialog.h"
///#include "abstractsettings_p.h"
///
///#include "iconloader_p.h"
///#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QPointer>

#include <QtGui/QCursor>
#include <QtGui/QFontDatabase>
#include <QtGui/QIcon>
#include <QtGui/QMoveEvent>
#include <QtGui/QPainter>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QTextDocument>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    #include <QtGui/QAction>
#else
    #include <QtWidgets/QAction>
#endif
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QMenu>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidgetAction>

///static const char *RichTextDialogC = "RichTextDialog";
///static const char *Geometry = "Geometry";

class RichTextEditor : public QTextEdit
{
    Q_OBJECT
public:
    RichTextEditor(QWidget *parent = 0);
    void setDefaultFont(const QFont &font);

    QToolBar *createToolBar(QWidget *parent = 0);

Q_SIGNALS:
    void wheelScrolled(int steps); // +fotowall

private:
    void wheelEvent(QWheelEvent * e); // +fotowall

public slots:
    void setFontBold(bool b);
    void setFontPointSize(double);
    void setText(const QString &text);
    QString text(Qt::TextFormat format) const;

signals:
    void stateChanged();
};
/**
class AddLinkDialog : public QDialog
{
    Q_OBJECT

public:
    AddLinkDialog(RichTextEditor *editor, QWidget *parent = 0);
    ~AddLinkDialog();

    int showDialog();

public slots:
    void accept();

private:
    RichTextEditor *m_editor;
    Ui::AddLinkDialog *m_ui;
};

AddLinkDialog::AddLinkDialog(RichTextEditor *editor, QWidget *parent) :
    QDialog(parent),
    m_ui(new Ui::AddLinkDialog)
{
    m_ui->setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_editor = editor;
}

AddLinkDialog::~AddLinkDialog()
{
    delete m_ui;
}

int AddLinkDialog::showDialog()
{
    // Set initial focus
    const QTextCursor cursor = m_editor->textCursor();
    if (cursor.hasSelection()) {
        m_ui->titleInput->setText(cursor.selectedText());
        m_ui->urlInput->setFocus();
    } else {
        m_ui->titleInput->setFocus();
    }

    return exec();
}

void AddLinkDialog::accept()
{
    const QString title = m_ui->titleInput->text();
    const QString url = m_ui->urlInput->text();

    if (!title.isEmpty()) {
        QString html = QLatin1String("<a href=\"");
        html += url;
        html += QLatin1String("\">");
        html += title;
        html += QLatin1String("</a>");

        m_editor->insertHtml(html);
    }

    m_ui->titleInput->clear();
    m_ui->urlInput->clear();

    QDialog::accept();
}
**/
class HtmlTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    HtmlTextEdit(QWidget *parent = 0)
        : QTextEdit(parent)
    {}

    void contextMenuEvent(QContextMenuEvent *event);

private slots:
    void actionTriggered(QAction *action);
};

void HtmlTextEdit::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu *menu = createStandardContextMenu();
    QMenu *htmlMenu = new QMenu(tr("Insert HTML entity"), menu);

    typedef struct {
        const char *text;
        const char *entity;
    } Entry;

    const Entry entries[] = {
        { "&&amp; (&&)", "&amp;" },
        { "&&nbsp;", "&nbsp;" },
        { "&&lt; (<)", "&lt;" },
        { "&&gt; (>)", "&gt;" },
        { "&&copy; (Copyright)", "&copy;" },
        { "&&reg; (Trade Mark)", "&reg;" },
    };

    for (int i = 0; i < 6; ++i) {
        QAction *entityAction = new QAction(QLatin1String(entries[i].text),
                                            htmlMenu);
        entityAction->setData(QLatin1String(entries[i].entity));
        htmlMenu->addAction(entityAction);
    }

    menu->addMenu(htmlMenu);
    connect(htmlMenu, SIGNAL(triggered(QAction*)),
                      SLOT(actionTriggered(QAction*)));
    menu->exec(event->globalPos());
    delete menu;
}

void HtmlTextEdit::actionTriggered(QAction *action)
{
    insertPlainText(action->data().toString());
}
/*
class ColorAction : public QAction
{
    Q_OBJECT

public:
    ColorAction(QObject *parent);

    const QColor& color() const { return m_color; }
    void setColor(const QColor &color);

signals:
    void colorChanged(const QColor &color);

private slots:
    void chooseColor();

private:
    QColor m_color;
};

ColorAction::ColorAction(QObject *parent):
    QAction(parent)
{
    setText(tr("Text Color"));
    setColor(Qt::black);
    connect(this, SIGNAL(triggered()), this, SLOT(chooseColor()));
}

void ColorAction::setColor(const QColor &color)
{
    m_color = color;
    QPixmap pix(24, 24);
    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.fillRect(pix.rect(), m_color);
    painter.setPen(m_color.darker());
    painter.drawRect(pix.rect().adjusted(0, 0, -1, -1));
    setIcon(pix);
}

void ColorAction::chooseColor()
{
    const QColor col = QColorTriangleDialog::getColor(m_color, 0, tr("Select Text Color"),
        QColorDialog::ShowAlphaChannel | QColorDialog::DontUseNativeDialog);
    if (col.isValid() && col != m_color) {
        setColor(col);
        emit colorChanged(m_color);
    }
}
*/
class RichTextEditorToolBar : public QToolBar
{
    Q_OBJECT
public:
    RichTextEditorToolBar(RichTextEditor *editor,
                          QWidget *parent = 0);

public slots:
    void updateActions();

private slots:
    ///void alignmentActionTriggered(QAction *action);
    void sizeInputActivated(const QString &size);
    void fontInputActivated(const QString &face);   // +fotowall
    void colorChanged(const QColor &color);
    void setVAlignSuper(bool super);
    void setVAlignSub(bool sub);
    ///void insertLink();
    ///void insertImage();
    void slotWheelScrolled(int steps);    // +fotowall

protected: // +fotowall
    void paintEvent(QPaintEvent * event);   // +fotowall

private:
    QAction *m_bold_action;
    QAction *m_italic_action;
    QAction *m_underline_action;
    QAction *m_valign_sup_action;
    QAction *m_valign_sub_action;
    ///QAction *m_align_left_action;
    ///QAction *m_align_center_action;
    ///QAction *m_align_right_action;
    ///QAction *m_align_justify_action;
    ///QAction *m_link_action;
    ///QAction *m_image_action;
    ///ColorAction *m_color_action;
    QComboBox *m_font_size_input;

    QFontComboBox *m_font_face_input;   // +fotowall

    QPointer<RichTextEditor> m_editor;
};

static QAction *createCheckableAction(const QIcon &icon, const QString &text,
                                      QObject *receiver, const char *slot,
                                      QObject *parent = 0)
{
    QAction *result = new QAction(parent);
    result->setIcon(icon);
    result->setText(text);
    result->setCheckable(true);
    result->setChecked(false);
    if (slot)
        QObject::connect(result, SIGNAL(triggered(bool)), receiver, slot);
    return result;
}

RichTextEditorToolBar::RichTextEditorToolBar(RichTextEditor *editor,
                                             QWidget *parent) :
    QToolBar(parent),
    ///m_link_action(new QAction(this)),
    ///m_image_action(new QAction(this)),
    ///m_color_action(new ColorAction(this)),
    m_font_size_input(new QComboBox),
    m_font_face_input(new QFontComboBox(this)), // +fotowall
    m_editor(editor)
{
    // Text color button
    ///connect(m_color_action, SIGNAL(colorChanged(QColor)),
    ///        this, SLOT(colorChanged(QColor)));
    ///addAction(m_color_action);

    ///addSeparator();

    // Font size combo box
    m_font_size_input->setEditable(false);

    QList<int> font_sizes = QFontDatabase::standardSizes(); // +fotowall
    qreal lastSize = font_sizes.last();
    qreal magFactor = 1.1892;
    while (font_sizes.last() < 400) {   // +fotowall
        font_sizes.append((int)(lastSize * magFactor));
        magFactor *= 1.1892;
    }

    foreach (int font_size, font_sizes)
        m_font_size_input->addItem(QString::number(font_size));

    connect(m_font_size_input, SIGNAL(activated(QString)),
            this, SLOT(sizeInputActivated(QString)));
    addWidget(m_font_size_input);

    // Font face combo box  // +fotowall
    m_font_face_input->setMaximumWidth(120);
    m_font_face_input->setEditable(false);
    connect(m_font_face_input, SIGNAL(activated(QString)),
            this, SLOT(fontInputActivated(QString)));
    addWidget(m_font_face_input);

    ///addSeparator();

    // Bold, italic and underline buttons

    m_bold_action = createCheckableAction(
            createIconSet(QLatin1String("textbold.png")),
            tr("Bold"), editor, SLOT(setFontBold(bool)), this);
    m_bold_action->setShortcut(tr("CTRL+B"));
    addAction(m_bold_action);

    m_italic_action = createCheckableAction(
            createIconSet(QLatin1String("textitalic.png")),
            tr("Italic"), editor, SLOT(setFontItalic(bool)), this);
    m_italic_action->setShortcut(tr("CTRL+I"));
    addAction(m_italic_action);

    m_underline_action = createCheckableAction(
            createIconSet(QLatin1String("textunder.png")),
            tr("Underline"), editor, SLOT(setFontUnderline(bool)), this);
    m_underline_action->setShortcut(tr("CTRL+U"));
    addAction(m_underline_action);

    ///addSeparator();

    /**
    // Left, center, right and justified alignment buttons

    QActionGroup *alignment_group = new QActionGroup(this);
    connect(alignment_group, SIGNAL(triggered(QAction*)),
                             SLOT(alignmentActionTriggered(QAction*)));

    m_align_left_action = createCheckableAction(
            createIconSet(QLatin1String("textleft.png")),
            tr("Left Align"), editor, 0, alignment_group);
    addAction(m_align_left_action);

    m_align_center_action = createCheckableAction(
            createIconSet(QLatin1String("textcenter.png")),
            tr("Center"), editor, 0, alignment_group);
    addAction(m_align_center_action);

    m_align_right_action = createCheckableAction(
            createIconSet(QLatin1String("textright.png")),
            tr("Right Align"), editor, 0, alignment_group);
    addAction(m_align_right_action);

    m_align_justify_action = createCheckableAction(
            createIconSet(QLatin1String("textjustify.png")),
            tr("Justify"), editor, 0, alignment_group);
    addAction(m_align_justify_action);

    addSeparator();
    */

    // Superscript and subscript buttons

    m_valign_sup_action = createCheckableAction(
            createIconSet(QLatin1String("textsuperscript.png")),
            tr("Superscript"),
            this, SLOT(setVAlignSuper(bool)), this);
    ///addAction(m_valign_sup_action);

    m_valign_sub_action = createCheckableAction(
            createIconSet(QLatin1String("textsubscript.png")),
            tr("Subscript"),
            this, SLOT(setVAlignSub(bool)), this);
    ///addAction(m_valign_sub_action);

    ///addSeparator();

    // Insert hyperlink and image buttons
///    m_link_action->setIcon(createIconSet(QLatin1String("textanchor.png")));
///    m_link_action->setText(tr("Insert &Link"));
///    connect(m_link_action, SIGNAL(triggered()), SLOT(insertLink()));
///    addAction(m_link_action);

/** Commented because the new Text Rendering algorithm doesn't support that
    m_image_action->setIcon(createIconSet(QLatin1String("insertimage.png")));
    m_image_action->setText(tr("Insert &Image"));
    connect(m_image_action, SIGNAL(triggered()), SLOT(insertImage()));
    addAction(m_image_action);
*/

    connect(editor, SIGNAL(wheelScrolled(int)), this, SLOT(slotWheelScrolled(int)));    // +fotowall
    connect(editor, SIGNAL(textChanged()), this, SLOT(updateActions()));
    connect(editor, SIGNAL(stateChanged()), this, SLOT(updateActions()));

    updateActions();
}
/**
void RichTextEditorToolBar::alignmentActionTriggered(QAction *action)
{
    Qt::Alignment new_alignment;

    if (action == m_align_left_action) {
        new_alignment = Qt::AlignLeft;
    } else if (action == m_align_center_action) {
        new_alignment = Qt::AlignCenter;
    } else if (action == m_align_right_action) {
        new_alignment = Qt::AlignRight;
    } else {
        new_alignment = Qt::AlignJustify;
    }

    m_editor->setAlignment(new_alignment);
}
*/
void RichTextEditorToolBar::colorChanged(const QColor &color)
{
    m_editor->setTextColor(color);
    m_editor->setFocus();
}

void RichTextEditorToolBar::sizeInputActivated(const QString &size)
{
    bool ok;
    int i = size.toInt(&ok);
    if (!ok)
        return;

    m_editor->setFontPointSize(i);
    m_editor->setFocus();
}

void RichTextEditorToolBar::fontInputActivated(const QString &face) // +fotowall
{
    m_editor->setFontFamily(face);
    m_editor->setFocus();
}

void RichTextEditorToolBar::setVAlignSuper(bool super)
{
    const QTextCharFormat::VerticalAlignment align = super ?
        QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sub_action->setChecked(false);
}

void RichTextEditorToolBar::setVAlignSub(bool sub)
{
    const QTextCharFormat::VerticalAlignment align = sub ?
        QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal;

    QTextCharFormat charFormat = m_editor->currentCharFormat();
    charFormat.setVerticalAlignment(align);
    m_editor->setCurrentCharFormat(charFormat);

    m_valign_sup_action->setChecked(false);
}
/**
void RichTextEditorToolBar::insertLink()
{
    notImplemented();
    //AddLinkDialog linkDialog(m_editor, this);
    //linkDialog.showDialog();
    //sm_editor->setFocus();
}

void RichTextEditorToolBar::insertImage()
{
    ///const QString path = IconSelector::choosePixmapResource(m_core, m_core->resourceModel(), QString(), this);
    const QString path = QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Images (*.jpeg *.jpg *.png *.bmp *.tif *.tiff)"));
    if (!path.isEmpty())
        m_editor->insertHtml(QLatin1String("<img src=\"") + path + QLatin1String("\"/>"));
}
*/
void RichTextEditorToolBar::slotWheelScrolled(int steps)    // +fotowall
{
    // find out the new font index
    const int idx = m_font_size_input->currentIndex();
    if (idx == -1)
        return;
    int newIdx = idx + steps;
    if (newIdx < 0)
        newIdx = 0;
    else if (newIdx > m_font_size_input->count())
        newIdx = m_font_size_input->count();

    // change index
    m_font_size_input->setCurrentIndex(newIdx);
    sizeInputActivated(m_font_size_input->currentText());
}

void RichTextEditorToolBar::paintEvent(QPaintEvent * /*event*/) // +fotowall
{
    QPainter p(this);
    QColor windowColor = palette().brush(QPalette::Window).color();
    QLinearGradient lg(0, 0, 0, 1);
    lg.setCoordinateMode(QGradient::ObjectBoundingMode);
    lg.setColorAt(0.0, windowColor.lighter(150));
    lg.setColorAt(1.0, windowColor.darker(110));
    p.setCompositionMode(QPainter::CompositionMode_Source);
    p.fillRect(rect(), lg);
}

void RichTextEditorToolBar::updateActions()
{
    if (m_editor == 0) {
        setEnabled(false);
        return;
    }

//    const Qt::Alignment alignment = m_editor->alignment();
    const QTextCursor cursor = m_editor->textCursor();
    const QTextCharFormat charFormat = cursor.charFormat();
    const QFont font = charFormat.font();
    const QTextCharFormat::VerticalAlignment valign =
        charFormat.verticalAlignment();
    const bool superScript = valign == QTextCharFormat::AlignSuperScript;
    const bool subScript = valign == QTextCharFormat::AlignSubScript;
/**
    if (alignment & Qt::AlignLeft) {
        m_align_left_action->setChecked(true);
    } else if (alignment & Qt::AlignRight) {
        m_align_right_action->setChecked(true);
    } else if (alignment & Qt::AlignHCenter) {
        m_align_center_action->setChecked(true);
    } else {
        m_align_justify_action->setChecked(true);
    }
*/
    m_bold_action->setChecked(font.bold());
    m_italic_action->setChecked(font.italic());
    m_underline_action->setChecked(font.underline());
    m_valign_sup_action->setChecked(superScript);
    m_valign_sub_action->setChecked(subScript);

    const int size = font.pointSize();
    const int idx = m_font_size_input->findText(QString::number(size));
    if (idx != -1)
        m_font_size_input->setCurrentIndex(idx);

    const QString face = font.family(); // +fotowall
    const int fIdx = m_font_face_input->findText(face);
    if (fIdx != -1)
        m_font_face_input->setCurrentIndex(fIdx);

    ///m_color_action->setColor(m_editor->textColor());
}

RichTextEditor::RichTextEditor(QWidget *parent)
    : QTextEdit(parent)
{
    connect(this, SIGNAL(currentCharFormatChanged(QTextCharFormat)),
            this, SIGNAL(stateChanged()));
    connect(this, SIGNAL(cursorPositionChanged()),
            this, SIGNAL(stateChanged()));
}

QToolBar *RichTextEditor::createToolBar(QWidget *parent)
{
    return new RichTextEditorToolBar(this, parent);
}

void RichTextEditor::wheelEvent(QWheelEvent * e)    // +fotowall
{
    emit wheelScrolled(e->angleDelta().y() / 120);
}

void RichTextEditor::setFontBold(bool b)
{
    if (b)
        setFontWeight(QFont::Bold);
    else
        setFontWeight(QFont::Normal);
}

void RichTextEditor::setFontPointSize(double d)
{
    QTextEdit::setFontPointSize(qreal(d));
}

void RichTextEditor::setText(const QString &text)
{
    if (Qt::mightBeRichText(text))
        setHtml(text);
    else
        setPlainText(text);
}

void RichTextEditor::setDefaultFont(const QFont &font)
{
    document()->setDefaultFont(font);
    if (font.pointSize() > 0)
        setFontPointSize(font.pointSize());
    else
        setFontPointSize(QFontInfo(font).pointSize());
    emit textChanged();
}

QString RichTextEditor::text(Qt::TextFormat format) const
{
    switch (format) {
    case Qt::PlainText:
        return toPlainText();
    case Qt::RichText:
        return toHtml();
    case Qt::AutoText:
        break;
    }
    const QString html = toHtml();
    const QString plain = toPlainText();
    QTextEdit tester;
    tester.setPlainText(plain);
    return tester.toHtml() == html ? plain : html;
}

RichTextEditorDialog::RichTextEditorDialog(QWidget *parent)  :
    QDialog(parent),
    m_editor(new RichTextEditor),
    m_text_edit(new HtmlTextEdit),
    m_tab_widget(new QTabWidget),
    m_state(Clean)
{
    //setWindowTitle(tr("Edit text"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    m_text_edit->setAcceptRichText(false);
    ///new HtmlHighlighter(m_text_edit);

    connect(m_editor, SIGNAL(textChanged()), this, SLOT(richTextChanged()));
    connect(m_text_edit, SIGNAL(textChanged()), this, SLOT(sourceChanged()));

    // The toolbar needs to be created after the RichTextEditor
    QToolBar *tool_bar = m_editor->createToolBar(this);
    tool_bar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    m_color_tri = new QtColorTriangle;
    m_color_tri->setMinimumWidth(160);
    QPalette triPal;
    triPal.setBrush(QPalette::Mid, m_editor->palette().color(QPalette::Base).darker(110));
    m_color_tri->setPalette(triPal);
    connect(m_color_tri, SIGNAL(colorChanged(QColor)), m_editor, SLOT(setTextColor(QColor)));
    connect(m_editor, SIGNAL(textChanged()), this, SLOT(slotUpdateActions()));
    connect(m_editor, SIGNAL(stateChanged()), this, SLOT(slotUpdateActions()));

    QWidget *rich_edit = new QWidget;
    QGridLayout *rich_edit_layout = new QGridLayout(rich_edit);
    rich_edit_layout->setContentsMargins(0, 0, 0, 0);     // +fotowall
    rich_edit_layout->setSpacing(0);    // +fotowall
    rich_edit_layout->addWidget(tool_bar, 0, 0, 1, 2);
    rich_edit_layout->addWidget(m_editor, 1, 0, 1, 1);
    rich_edit_layout->addWidget(m_color_tri, 1, 1, 1, 1);

    QWidget *plain_edit = new QWidget;
    QVBoxLayout *plain_edit_layout = new QVBoxLayout(plain_edit);
    plain_edit_layout->setContentsMargins(0, 0, 0, 0);    // +fotowall
    plain_edit_layout->addWidget(m_text_edit);

    m_tab_widget->setTabPosition(QTabWidget::South);
    m_tab_widget->setDocumentMode(true); // +fotowall
    m_tab_widget->addTab(rich_edit, tr("Visual Editor"));
    m_tab_widget->addTab(plain_edit, tr("Source"));
    connect(m_tab_widget, SIGNAL(currentChanged(int)),
                          SLOT(tabIndexChanged(int)));

    /**QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
    QPushButton *ok_button = buttonBox->button(QDialogButtonBox::Ok);
    ok_button->setText(tr("&OK"));
    ok_button->setDefault(true);
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("&Cancel"));
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));*/

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);   // +fotowall
    layout->addWidget(m_tab_widget);
    ///layout->addWidget(buttonBox);

    m_editor->setFrameStyle(QFrame::NoFrame);   // +fotowall
    m_text_edit->setFrameStyle(QFrame::NoFrame);   // +fotowall
    m_editor->setFocusPolicy(Qt::StrongFocus);
    m_editor->setFocus();
}

void RichTextEditorDialog::focusEditor()
{
    m_editor->selectAll();
    m_editor->setFocus();
}

/*int RichTextEditorDialog::showDialog()
{
    m_tab_widget->setCurrentIndex(0);
    m_editor->selectAll();
    m_editor->setFocus();

    return exec();
}*/

void RichTextEditorDialog::setDefaultFont(const QFont &font)
{
    m_editor->setDefaultFont(font);
}

void RichTextEditorDialog::setText(const QString &text)
{
    m_editor->setText(text);
    m_text_edit->setPlainText(text);
    m_state = Clean;

    // move the cursor to refresh attributes
    QTextCursor cursor = m_editor->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_editor->setTextCursor(cursor);
}

QString RichTextEditorDialog::text(Qt::TextFormat format) const
{
    // In autotext mode, if the user has changed the source, use that
    if (format == Qt::AutoText && (m_state == Clean || m_state == SourceChanged))
        return m_text_edit->toPlainText();
    // If the plain text HTML editor is selected, first copy its contents over
    // to the rich text editor so that it is converted to Qt-HTML or actual
    // plain text.
    if (m_tab_widget->currentIndex() == SourceIndex && m_state == SourceChanged)
        m_editor->setHtml(m_text_edit->toPlainText());
    return m_editor->text(format);
}

void RichTextEditorDialog::tabIndexChanged(int newIndex)
{
    // Anything changed, is there a need for a conversion?
    if (newIndex == SourceIndex && m_state != RichTextChanged)
        return;
    if (newIndex == RichTextIndex && m_state != SourceChanged)
        return;
    const State oldState = m_state;
    // Remember the cursor position, since it is invalidated by setPlainText
    QTextEdit *new_edit = (newIndex == SourceIndex) ? m_text_edit : m_editor;
    const int position = new_edit->textCursor().position();

    if (newIndex == SourceIndex)
        m_text_edit->setPlainText(m_editor->text(Qt::RichText));
    else
        m_editor->setHtml(m_text_edit->toPlainText());

    QTextCursor cursor = new_edit->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (cursor.position() > position) {
        cursor.setPosition(position);
    }
    new_edit->setTextCursor(cursor);
    m_state = oldState; // Changed is triggered by setting the text
}

void RichTextEditorDialog::richTextChanged()
{
    m_state = RichTextChanged;
}

void RichTextEditorDialog::sourceChanged()
{
    m_state = SourceChanged;
}

void RichTextEditorDialog::slotUpdateActions()
{
    m_color_tri->animateSetColor(m_editor->textColor());
}

#include "richtexteditor.moc"
