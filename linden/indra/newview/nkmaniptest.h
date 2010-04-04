/*
 * NKManipTest.h
 *
 *  Created on: 2010-3-31
 *      Author: nekoyasha
 */

#ifndef NKMANIPTEST_H_
#define NKMANIPTEST_H_

#include "llmanip.h"

#include "llviewerobject.h"

class LLToolComposite;

class NKManipTest : public LLManip, public LLSingleton<NKManipTest> {
public:
	NKManipTest();
	virtual ~NKManipTest();
	
	virtual BOOL handleMouseDownOnPart(S32 x, S32 y, MASK mask){return TRUE;}
	virtual void highlightManipulators(S32 x, S32 y){;}
	virtual BOOL canAffectSelection() {return FALSE;}


	virtual void handleSelect();
	virtual void handleDeselect();
	virtual void render();
	virtual void draw();

protected:
	LLViewerObject* mvo;
};

#endif /* NKMANIPTEST_H_ */
