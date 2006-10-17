/* This file is part of the KDE project
   Copyright (C) 2006 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/
#ifndef KEXILOOKUPCOLUMNPAGE_H
#define KEXILOOKUPCOLUMNPAGE_H

#include <qwidget.h>
#include <kexidb/field.h>
#include <kexidb/utils.h>
#include <koproperty/set.h>

class KCommand;
class KexiObjectInfoLabel;
class KexiDataSourceComboBox;
class KexiFieldComboBox;
class KexiFieldListView;
class KexiProject;
class KexiSmallToolButton;
class QToolButton;
class QLabel;
class QFrame;

//! @short A page within table designer's property pane, providing lookup column editor.
/*! It's data model is basically KexiDB::LookupFieldSchema class, but the page does 
 not create it directly but instead updates a property set that defines 
 the field currently selected in the designer. 
 
 @todo not all features of KexiDB::LookupFieldSchema class are displayed on this page yet
 */
class KexiLookupColumnPage : public QWidget
{
	Q_OBJECT

	public:
		KexiLookupColumnPage(QWidget *parent);
		virtual ~KexiLookupColumnPage();

	public slots:
		void setProject(KexiProject *prj);
		void clearRowSourceSelection(bool alsoClearComboBox = true);
		void clearBoundColumnSelection();
		void clearVisibleColumnSelection();

		//! Receives a pointer to a new property \a set (from KexiFormView::managerPropertyChanged())
		void assignPropertySet(KoProperty::Set* propertySet);

	signals:
		//! Signal emitted when helper button 'Go to selected row sourcesource' is clicked.
		void jumpToObjectRequested(const QCString& mime, const QCString& name);

//		/*! Signal emitted when current bound column has been changed. */
//		void boundColumnChanged(const QString& string, const QString& caption,
	//		KexiDB::Field::Type type);

	protected slots:
		void slotRowSourceTextChanged(const QString & string);
		void slotRowSourceChanged();
		void slotGotoSelectedRowSource();
		void slotBoundColumnSelected();
		void slotVisibleColumnSelected();

	protected:
		void updateBoundColumnWidgetsAvailability();

		//! Used instead of m_propertySet->changeProperty() to honor m_propertySetEnabled
		void changeProperty(const QCString &property, const QVariant &value);

//MOC_SKIP_BEGIN
#if 0
		KexiDataSourceComboBox* dataSourceCombo() const { return m_dataSourceCombo; }
		KexiObjectInfoLabel* objectInfoLabel() const { return m_objectInfoLabel; }

	public slots:

		//! Sets data source of a currently selected form. 
		//! This is performed on form initialization and on activating.
		void setDataSource(const QCString& mimeType, const QCString& name);

	signals:
		//! Signal emitted when form's data source has been changed. It's connected to the Form Manager.
		void formDataSourceChanged(const QCString& mime, const QCString& name);

		/*! Signal emitted when 'insert fields' button has been clicked */
		void insertAutoFields(const QString& sourceMimeType, const QString& sourceName,
			const QStringList& fields);

	protected slots:
		void slotInsertSelectedFields();
		void slotFieldListViewSelectionChanged();
		void slotFieldDoubleClicked(const QString& sourceMimeType, const QString& sourceName,
			const QString& fieldName);

	protected:
#endif
//MOC_SKIP_END

	private:
		class Private;
		Private* d;
};

#endif
