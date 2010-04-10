/*
 * NKManipProfileCut.h
 *
 *  Created on: 2010-4-6
 *      Author: nekoyasha
 */

#ifndef NKMANIPPROFILECUT_H_
#define NKMANIPPROFILECUT_H_

#include "llmanip.h"

#include "llviewerobject.h"
#include "llvolume.h"

class LLToolComposite;
class LLPickInfo;

class NKManipProfileCut: public LLManip, public LLSingleton<NKManipProfileCut >
{
public:
	NKManipProfileCut();
	virtual ~NKManipProfileCut();

	virtual BOOL handleMouseDownOnPart(S32 x, S32 y, MASK mask){return TRUE;}
	virtual void highlightManipulators(S32 x, S32 y){;}
	virtual BOOL canAffectSelection() {return FALSE;}


	virtual void handleSelect();
	virtual void handleDeselect();
	virtual void render();
	virtual void draw();

	virtual BOOL handleMouseDown(S32 x, S32 y, MASK mask);
	virtual BOOL handleMouseUp(S32 x, S32 y, MASK mask);
	virtual BOOL handleHover(S32 x, S32 y, MASK mask);

	//*HACK
	static void pickCallback(const LLPickInfo& pick_info);

protected:
	LLViewerObject* mvo;

	std::vector<LLVector3> mManipPath;
	std::vector< std::vector<LLVector3> > mManipParts;
	F32 mSelectedPosition;	//0.0f-1.0f, where is the mouse on the particular part
							//NOTE: Does *not* correspond to full path/profile, only the cut part

	void generateManipulators();
	void generateManipulatorPaths();
	void updateProximity(S32 x, S32 y);
	void drag(S32 x, S32 y, MASK mask);
	void updateSelection();

	LLVolumeParams mLastParams; /// hold params for checking object updates && drag
	F32 mLastDragCoeff;
};

#endif /* NKMANIPPROFILECUT_H_ */
