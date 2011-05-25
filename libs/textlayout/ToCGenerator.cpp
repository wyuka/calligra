/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 Jean Nicolas Artaud <jean.nicolas.artaud@kogmbh.com>
 * Copyright (C) 2011 Pavol Korinek <pavol.korinek@ixonos.com>
 * Copyright (C) 2011 Lukáš Tvrdý <lukas.tvrdy@ixonos.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "ToCGenerator.h"
#include <klocale.h>

#include "KoTextDocumentLayout.h"
#include "KoTextLayoutRootArea.h"
#include "KoTextShapeData.h"
#include "ToCDocumentLayout.h"

#include <KoParagraphStyle.h>
#include <KoTextPage.h>
#include <KoShape.h>
#include <KoTextDocument.h>
#include <KoTextBlockData.h>
#include <KoStyleManager.h>
#include <KoTextEditor.h>

#include <QTextDocument>
#include <QTimer>
#include <KDebug>
#include <KoBookmark.h>
#include <KoInlineTextObjectManager.h>

static const QString INVALID_HREF_TARGET = "INVALID_HREF";

ToCGenerator::ToCGenerator(QTextDocument *tocDocument, QTextBlock block, KoTableOfContentsGeneratorInfo *tocInfo)
    : QObject(tocDocument)
    , m_ToCDocument(tocDocument)
    , m_ToCInfo(tocInfo)
    , m_block(block)
    , m_generatedDocumentChangeCount(-1)
{
    Q_ASSERT(tocDocument);
    Q_ASSERT(tocInfo);

    m_ToCInfo->setGenerator(this);

    tocDocument->setUndoRedoEnabled(false);
    tocDocument->setDocumentLayout(new ToCDocumentLayout(tocDocument));

    m_documentLayout = static_cast<KoTextDocumentLayout *>(m_block.document()->documentLayout());
    m_document = m_documentLayout->document();

    // connect to FinishedLayout
    connect(m_documentLayout, SIGNAL(finishedLayout()), this, SLOT(generate()));

    // do a generate right now to have a ToC with placeholder numbers.
    generate();
    m_generatedDocumentChangeCount = -1; // we need one more intial layout to get pagenumbers
}

ToCGenerator::~ToCGenerator()
{
    delete m_ToCInfo;
}


static KoParagraphStyle *generateTemplateStyle(KoStyleManager *styleManager, int outlineLevel) {
    KoParagraphStyle *style = new KoParagraphStyle();
    style->setName("Contents " + QString::number(outlineLevel));
    style->setParent(styleManager->paragraphStyle("Standard"));
    style->setLeftMargin(QTextLength(QTextLength::FixedLength, (outlineLevel - 1) * 8));
    styleManager->add(style);
    return style;
}

QString ToCGenerator::fetchBookmarkRef(QTextBlock block, KoInlineTextObjectManager* inlineTextObjectManager)
{
    QTextBlock::iterator it;
    for (it = block.begin(); !(it.atEnd()); ++it) {
        QTextFragment currentFragment = it.fragment();
        // most possibly inline object
        if (currentFragment.text()[0].unicode() == QChar::ObjectReplacementCharacter && currentFragment.isValid()) {
            KoInlineObject *inlineObject = inlineTextObjectManager->inlineTextObject( currentFragment.charFormat() );
            KoBookmark *bookmark = dynamic_cast<KoBookmark*>(inlineObject);
            if (bookmark) {
                return bookmark->name();
                break;
            }
        }
    }

    return QString();
}


static QString removeWhitespacePrefix(const QString& text)
{
    int firstNonWhitespaceCharIndex = 0;
    int lenght = text.length();
    while (firstNonWhitespaceCharIndex < lenght && text.at(firstNonWhitespaceCharIndex).isSpace()) {
        firstNonWhitespaceCharIndex++;
    }
    return text.right(lenght - firstNonWhitespaceCharIndex);
}


void ToCGenerator::generate()
{
    if (!m_ToCInfo)
        return;

    if (m_documentLayout->documentChangedCount() == m_generatedDocumentChangeCount) {
        return;
    }


    QTextCursor cursor = m_ToCDocument->rootFrame()->lastCursorPosition();
    cursor.setPosition(m_ToCDocument->rootFrame()->firstPosition(), QTextCursor::KeepAnchor);
    cursor.beginEditBlock();

    KoStyleManager *styleManager = KoTextDocument(m_document).styleManager();

    if (!m_ToCInfo->m_indexTitleTemplate.text.isNull()) {
        KoParagraphStyle *titleStyle = styleManager->paragraphStyle(m_ToCInfo->m_indexTitleTemplate.styleId);
        if (!titleStyle) {
            titleStyle = styleManager->defaultParagraphStyle();
        }

        QTextBlock titleTextBlock = cursor.block();
        titleStyle->applyStyle(titleTextBlock);

        cursor.insertText(m_ToCInfo->m_indexTitleTemplate.text);
        cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
    }

    // Add TOC
    // Iterate through all blocks to generate TOC
    QTextBlock block = m_document->rootFrame()->firstCursorPosition().block();
    int blockId = 0;
    while (block.isValid()) {
        QString tocEntryText = block.text();
        tocEntryText.remove(QChar::ObjectReplacementCharacter);
        // some headings contain tabs, replace all occurences with spaces
        tocEntryText.replace('\t',' ');
        tocEntryText = removeWhitespacePrefix(tocEntryText);
        // Choose only TOC blocks -- headings with outline level
        if (block.blockFormat().hasProperty(KoParagraphStyle::OutlineLevel) && !tocEntryText.isEmpty()) {
            cursor.insertBlock(QTextBlockFormat(), QTextCharFormat());
            int outlineLevel = block.blockFormat().intProperty(KoParagraphStyle::OutlineLevel);

            KoParagraphStyle *tocTemplateStyle = 0;
            if (outlineLevel >= 1 && (outlineLevel-1) < m_ToCInfo->m_entryTemplate.size()
                        && outlineLevel <= m_ToCInfo->m_useOutlineLevel) {
                // List's index starts with 0, outline level starts with 0
                TocEntryTemplate tocEntryTemplate = m_ToCInfo->m_entryTemplate.at(outlineLevel - 1);
                // ensure that we fetched correct entry template
                Q_ASSERT(tocEntryTemplate.outlineLevel == outlineLevel);
                if (tocEntryTemplate.outlineLevel != outlineLevel) {
                    qDebug() << "TOC outline level not found correctly " << outlineLevel;
                }

                tocTemplateStyle = styleManager->paragraphStyle(tocEntryTemplate.styleId);
                if (tocTemplateStyle == 0) {
                    tocTemplateStyle = generateTemplateStyle(styleManager, outlineLevel);
                }

                QTextBlock tocEntryTextBlock = cursor.block();
                tocTemplateStyle->applyStyle( tocEntryTextBlock );

                KoTextBlockData *bd = dynamic_cast<KoTextBlockData *>(block.userData());

                // save the current style due to hyperlinks
                QTextCharFormat savedCharFormat = cursor.charFormat();
                foreach (IndexEntry * entry,tocEntryTemplate.indexEntries) {
                    switch(entry->name) {
                        case IndexEntry::LINK_START: {
                            //IndexEntryLinkStart *linkStart = static_cast<IndexEntryLinkStart*>(entry);

                            QString target = fetchBookmarkRef(block, m_documentLayout->inlineTextObjectManager());

                            if (target.isNull()) {
                                // generate unique name for the bookmark
                                target = tocEntryText + "|outline" + QString::number(blockId);
                                blockId++;

                                // insert new KoBookmark
                                KoBookmark *bookmark = new KoBookmark(block.document());
                                bookmark->setName(target);
                                bookmark->setType(KoBookmark::SinglePosition);
                                QTextCursor blockCursor(block);
                                m_documentLayout->inlineTextObjectManager()->insertInlineObject(blockCursor, bookmark);
                            }

                            if (!target.isNull()) {
                                // copy it to alter subset of properties
                                QTextCharFormat linkCf(savedCharFormat);
                                linkCf.setAnchor(true);
                                linkCf.setAnchorHref('#'+ target);

                                QBrush foreground = linkCf.foreground();
                                foreground.setColor(Qt::blue);

                                linkCf.setForeground(foreground);
                                linkCf.setProperty(KoCharacterStyle::UnderlineStyle, KoCharacterStyle::SolidLine);
                                linkCf.setProperty(KoCharacterStyle::UnderlineType, KoCharacterStyle::SingleLine);
                                cursor.setCharFormat(linkCf);
                            }
                            break;
                        }
                        case IndexEntry::CHAPTER: {
                            //IndexEntryChapter *chapter = static_cast<IndexEntryChapter*>(entry);
                            if (bd) {
                                cursor.insertText(bd->counterText());
                            }
                            break;
                        }
                        case IndexEntry::SPAN: {
                            IndexEntrySpan *span = static_cast<IndexEntrySpan*>(entry);
                            cursor.insertText(span->text);
                            break;
                        }
                        case IndexEntry::TEXT: {
                            //IndexEntryText *text = static_cast<IndexEntryText*>(entry);
                            cursor.insertText(tocEntryText);
                            break;
                        }
                        case IndexEntry::TAB_STOP: {
                            cursor.insertText("\t");
                            break;
                        }
                        case IndexEntry::PAGE_NUMBER: {
                            //IndexEntryPageNumber *pageNumber = static_cast<IndexEntryPageNumber*>(entry);
                            cursor.insertText(resolvePageNumber(block));
                            break;
                        }
                        case IndexEntry::LINK_END: {
                            //IndexEntryLinkEnd *linkEnd = static_cast<IndexEntryLinkEnd*>(entry);
                            cursor.setCharFormat(savedCharFormat);
                            break;
                        }
                        default:{
                            qDebug() << "New or unknown index entry";
                            break;
                        }
                    }
                }// foreach
                cursor.setCharFormat(savedCharFormat);   // restore the cursor char format
            } else {
                qDebug() << "Invalid outline level: " << outlineLevel;
            }
        }
        block = block.next();
    }
    cursor.endEditBlock();
    KoTextLayoutRootArea *rootArea = m_documentLayout->rootAreaForPosition(m_block.position());

    if (rootArea) {
        rootArea->setDirty();
    }
    m_generatedDocumentChangeCount = m_documentLayout->documentChangedCount();
}


QString ToCGenerator::resolvePageNumber(const QTextBlock &headingBlock)
{
    KoTextDocumentLayout *layout = qobject_cast<KoTextDocumentLayout*>(m_document->documentLayout());
    KoTextLayoutRootArea *rootArea = layout->rootAreaForPosition(headingBlock.position());
    if (rootArea) {
        if (rootArea->page()) {
            return QString::number(rootArea->page()->pageNumber());
        }
    }
    return "###";
}

#include <ToCGenerator.moc>
