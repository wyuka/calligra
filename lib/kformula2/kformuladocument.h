/* This file is part of the KDE project
   Copyright (C) 2001 Andrea Rizzi <rizzi@kde.org>
	              Ulrich Kuettler <ulrich.kuettler@mailbox.tu-dresden.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2.
 
   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.
 
   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __KFORMULADOCUMENT_H
#define __KFORMULADOCUMENT_H

#include <qobject.h>

#include <kaction.h>
#include <kcommand.h>

#include "symboltable.h"

class KFormulaContainer;
class KFormulaWidget;


/**
 * A document that can contain a lot of formulas.
 */
class KFormulaDocument : public QObject {
    Q_OBJECT
    
public:

    /**
     * Creates a formula document with actions.
     *
     * @param collection a place to put the document's actions.
     * @param history the undo stack to use. Creates its own if zero.
     */
    KFormulaDocument(KActionCollection* collection, KCommandHistory* history = 0);

    /**
     * Creates a formula document that doesn't use actions.
     *
     * @param history the undo stack to use. Creates its own if zero.
     */
    KFormulaDocument(KCommandHistory* history = 0);
    
    ~KFormulaDocument();

    /**
     * Creates a new formula. The whole idea of the formula document
     * is to contain formulas.
     *
     * The new formula is not owned by the document so you are responsible
     * to delete in properly.
     */
    KFormulaContainer* createFormula();

    /**
     * Sets a new formula.
     */
    void activate(KFormulaContainer* formula);

    /**
     * Tells that a formula is about to vanish. This must not be
     * the active formula from now on.
     */
    void formulaDies(KFormulaContainer* formula);

    /**
     * @returns our undo stack so the formulas can use it.
     */
    KCommandHistory* getHistory() const { return history; }

    void undo();
    void redo();

    /**
     * @returns the documents symbol table
     */
    const SymbolTable& getSymbolTable() const { return table; }

    KAction* getAddBracketAction()       { return addBracketAction; }
    KAction* getAddFractionAction()      { return addFractionAction; }
    KAction* getAddRootAction()          { return addRootAction; }
    KAction* getAddSumAction()           { return addSumAction; }
    KAction* getAddProductAction()       { return addProductAction; }
    KAction* getAddIntegralAction()      { return addIntegralAction; }
    KAction* getAddMatrixAction()        { return addMatrixAction; }
    KAction* getAddUpperLeftAction()     { return addUpperLeftAction; }
    KAction* getAddLowerLeftAction()     { return addLowerLeftAction; }
    KAction* getAddUpperRightAction()    { return addUpperRightAction; }
    KAction* getAddLowerRightAction()    { return addLowerRightAction; }
    KAction* getAddGenericUpperAction()  { return addGenericUpperAction; }
    KAction* getAddGenericLowerAction()  { return addGenericLowerAction; }
    KAction* getRemoveEnclosingAction()  { return removeEnclosingAction; }
    KSelectAction* getLeftBracketAction()  { return leftBracket; }
    KSelectAction* getRightBracketAction() { return rightBracket; }

public slots:

    void paste();
    void copy();
    void cut();

    void addDefaultBracket();
    void addFraction();
    void addRoot();
    void addIntegral();
    void addProduct();
    void addSum();
    void addMatrix();
    void addLowerLeftIndex();
    void addUpperLeftIndex();
    void addLowerRightIndex();
    void addUpperRightIndex();
    void addGenericLowerIndex();
    void addGenericUpperIndex();
    void removeEnclosing();
    void delimiterLeft();
    void delimiterRight();

private:

    /**
     * Creates the standard formula actions and puts them into
     * the collection.
     */
    void createActions(KActionCollection* collection);

    /**
     * @returns whether we have a formula that can get requests.
     */
    bool hasFormula();
    
    // We know our actions, maybe a client is interessted...
    
    KAction* addBracketAction;
    KAction* addFractionAction;
    KAction* addRootAction;
    KAction* addSumAction;
    KAction* addProductAction;
    KAction* addIntegralAction;
    KAction* addMatrixAction;
    KAction* addUpperLeftAction;
    KAction* addLowerLeftAction;
    KAction* addUpperRightAction;
    KAction* addLowerRightAction;
    KAction* addGenericUpperAction;
    KAction* addGenericLowerAction;
    KAction* removeEnclosingAction;

    KSelectAction* leftBracket;
    KSelectAction* rightBracket;

    char leftBracketChar;
    char rightBracketChar;
    
    /**
     * The active formula.
     */
    KFormulaContainer* formula;

    /**
     * Our undo stack. We don't own it. The stack belongs to
     * our parent and might contain not formula related commands
     * as well.
     */
    KCommandHistory* history;

    /**
     * Tells whether we are responsible to remove our history.
     */
    bool ownHistory;

    /**
     * The symbols/names that are "known" to the system.
     */
    SymbolTable table;
};

#endif // __KFORMULADOCUMENT_H
