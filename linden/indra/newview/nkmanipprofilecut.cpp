/**
 * @file ManipTest.cpp
 *
 *  Created on: 2010-3-31
 *      Author: nekoyasha
 */
#include "llviewerprecompiledheaders.h"

#include <string>

#include "nkmanipprofilecut.h"
	//also includes llviewerobject.h

#include "llgl.h" //use gGL instead of directly calling GL to buffer up calls
#include "llselectmgr.h"
#include "llbbox.h" //already included in llselectmgr.h
#include "llagent.h"
#include "llvolume.h"
#include "llviewerwindow.h"
#include "v2math.h"
#include "llviewercamera.h"
#include "llmath.h"
#include "llquaternion.h"

//*TODO scale apparent width by UI factor?
const F32 MANIPULATOR_WIDTH = 1.8f;
const F32 SELECTED_MANIPULATOR_WIDTH = 3.6f;
const F32 MANIPULATOR_HOTSPOT_WIDTH = 6.0f; //*TODO make this a setting
const F32 MANIPULATOR_PATH_WIDTH = 1.0f;
const F32 MANIPULATOR_SCALE_HALF_LIFE = 0.07f; //unused
const S32 NUM_MANIPULATORS = 6; //NK_PROFILE_END_TOP - NK_PROFILE_BEGIN + 1


NKManipProfileCut::NKManipProfileCut()
: LLManip (std::string("Profile Cut"), NULL),
  mLastParams()
{	for(S32 i=0; i<NUM_MANIPULATORS; i++){
		std::vector<LLVector3>* vectorp = new std::vector<LLVector3>;
		mManipParts.push_back(*vectorp);
	}
}

NKManipProfileCut::~NKManipProfileCut() {
	// TODO will this take care of all the destructors?
}

void NKManipProfileCut::handleSelect(){
	//*TODO separate the "new object" logic from here
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	//*TODO this might not be the most descriptive name here...
	mvo = mObjectSelection -> getFirstObject();
	mHighlightedPart = LL_NO_PART;
	generateManipulators();
}

void NKManipProfileCut::handleDeselect(){
	//*TODO free stoff so there is no mem leak
}

BOOL NKManipProfileCut::handleMouseDown(S32 x, S32 y, MASK mask)
{
	//first do normal hover
	handleHover(x,y,mask);
	if (mHighlightedPart == LL_NO_PART){
		//didn't hit manipulators, select objects instead
		//bad idea to pick on mouse DOWN though
		gViewerWindow->pickAsync(x, y, mask, pickCallback);
		return TRUE;
	} else {
		if (!mvo || !(mvo->getVolume())) {return FALSE;}
		//if there is an active manipulator, enter drag
		setMouseCapture(TRUE);
		mLastParams.copyParams(mvo->getVolume()->getParams());
		generateManipulatorPaths();
		return TRUE;
		//*TODO test if in proxymity of manipulators
	}




}
BOOL NKManipProfileCut::handleMouseUp(S32 x, S32 y, MASK mask)
{
	//first do normal hover
	handleHover(x, y, mask);

	//drag in process?
	if(hasMouseCapture()){
		setMouseCapture(FALSE);
	}
		//release it
		//send stuff to server


	return TRUE;
}
BOOL NKManipProfileCut::handleHover(S32 x, S32 y, MASK mask)
{
	//todo
	if (hasMouseCapture())
	{
		drag(x,y, mask);
	}
	else
	{
		updateProximity(x, y);
	}
	return TRUE;
}
///Pick callback.
///
///mostly copied from LLToolIndividual::pickCallback() ...
void NKManipProfileCut::pickCallback(const LLPickInfo& pick_info)
{
	LLViewerObject* obj = pick_info.getObject();
	LLSelectMgr::getInstance()->deselectAll();
	if(obj)
	{
		//*TODO separate the "new object" logic from here
		LLSelectMgr::getInstance()->selectObjectOnly(obj);
		//*HACK
		//NKManipProfileCut::getInstance()->mvo=obj;

	}

}

void NKManipProfileCut::draw(){

	//gsomething -> setup2DRender();
	;
}

//render whatever is to be rendered
void NKManipProfileCut::render(){
	//are we in a drag?
	if (!hasMouseCapture())
	{
		//start by polling the selection manager.
		//*TODO: we really should be checking for LLVolume / Pcode here
		LLViewerObject* VOp = LLSelectMgr::getInstance()->getEditSelection()-> getFirstObject();
		if (VOp != mvo)
		{
			mvo=VOp;
			if (mvo->getVolume())
			{
				mLastParams.copyParams(mvo->getVolume()->getParams());
				generateManipulators();
			}
		}
		else if (mvo -> getVolume() && mLastParams != mvo->getVolume()->getParams())
		{
			mLastParams.copyParams(mvo->getVolume()->getParams());
			generateManipulators();
		}
	}
	LLBBox bbox;


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (mObjectSelection->getSelectType() == SELECT_TYPE_HUD)
	{
		F32 zoom = gAgent.mHUDCurZoom;
		glScalef(zoom, zoom, zoom);
	}
	gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
	if(mvo){
		//*TODO: different colors for visible/hidden parts?
		LLGLDepthTest gls_depth(GL_FALSE);

		for(S32 i=0; i<NUM_MANIPULATORS; i++){
			//calculate line width for this particular part
			//*TODO: Optimize this with LLRender maybe?
			glLineWidth((mHighlightedPart == NK_PROFILE_MIN + i)? SELECTED_MANIPULATOR_WIDTH: MANIPULATOR_WIDTH);
			glColor4f(0.2f, 0.5f, 0.8f, 0.8f);
			glBegin(GL_LINE_STRIP);
			for (S32 j=0; j<mManipParts[i].size(); j++){
				glVertex3fv(mManipParts[i][j].mV);
			}
			glEnd();
		}
		if (hasMouseCapture()){
			// draw the manipulator path as well.
			glLineWidth(MANIPULATOR_PATH_WIDTH);
			glColor4f(0.5f, 0.5f, 0.5f, 0.8f);
			glBegin(GL_LINE_STRIP);
			for (S32 i=0; i<mManipPath.size(); i++){
				glVertex3fv(mManipPath[i].mV);
			}
			glEnd();
		}
	}

//
	//	if (mvo)
//	{
//		LLVolume* volumep = mvo->getVolume();
//		if (volumep){
//			LLPath* pathp = &(volumep->getPath());
//			const LLProfile* profilep = &(volumep->getProfile());
//
//			bbox = mvo->getBoundingBoxAgent();
//			LLVector3 pos = bbox.getPositionAgent();
//			LLQuaternion rot = bbox.getRotation();
//			LLVector3 scale = mvo->getScale();
//			F32 angle_radians, x, y, z;
//
//			glTranslatef(pos.mV[VX], pos.mV[VY], pos.mV[VZ]);
//
//			rot.getAngleAxis(&angle_radians, &x, &y, &z);
//			glRotatef(angle_radians * RAD_TO_DEG, x, y, z);
//
//			glScalef(scale.mV[VX], scale.mV[VY], scale.mV[VZ]);
//
//			glColor4f(0.2f, 0.5f, 0.8f, 0.8f);
//			glLineWidth(1.8f);
//
//			S32 sizeS = pathp->mPath.size();
//			glBegin(GL_LINE_STRIP);
//			for (S32 s = 0; s < sizeS; s++)
//			{
//				LLVector3& v = pathp->mPath[s].mPos;
//				glVertex3f(v.mV[VX], v.mV[VY], v.mV[VZ]);
//			}
//			glEnd();
//
//			glColor4f(0.8f, 0.5f, 0.2f, 0.8f);
//			S32 sizeT = profilep->mProfile.size();
//			glBegin(GL_LINE_STRIP);
//			for (S32 t = 0; t < sizeT; t++)
//			{
//				const LLVector3& v = profilep->mProfile[t];
//				//not sure what the Z value is used for...
//				glVertex2f(v.mV[VX], v.mV[VY]);
//			}
//			glEnd();
//		}
//	}
	glPopMatrix();
	;

	LLVector3 v(0.3,0.4,0.5);
	renderXYZ(v);
}
///generate the handles that the user grabs
void NKManipProfileCut::generateManipulators()
{
	LLVector3 pos;
	LLBBox bbox;
	if (mvo){
		LLVolume* volumep = mvo->getVolume();
		if (volumep){
			LLPath* pathp = &(volumep->getPath());
			const LLProfile* profilep = &(volumep->getProfile());

			bbox = mvo->getBoundingBoxAgent();
			LLVector3 pos_agent = bbox.getPositionAgent();
			LLQuaternion rot_agent = bbox.getRotation();
			LLVector3 scale_agent = mvo->getScale();

			S32 sizeS = pathp->mPath.size();
			S32 sizeT = profilep->mProfile.size();
			S32 sizeTOuter = profilep->getTotalOut();
			//only hollows have a TotalOut
			//lazily test for faces and not actual params
			if (sizeTOuter == 0)
			{
				if (volumep->mFaceMask & (LL_FACE_PROFILE_END)){
					sizeTOuter = sizeT - 1;	//idk why - Kaku
				} else {
					sizeTOuter = sizeT;
				}
			} else {
				sizeTOuter = sizeTOuter;
			}
			//sizeTOuter = sizeTOuter? sizeTOuter: sizeT;
			//Wobuffet? Wobuffet! Waa-buffet! - Kaku


			//	NK_PROFILE_START - point @ profile start * path
			mManipParts[0].clear();
			for (S32 s = 0; s < sizeS; s++){
				LLVector2 scale = pathp->mPath[s].mScale;
				LLQuaternion rot = pathp -> mPath[s]. mRot;

				//for(S32 t = 0; t < 1; t++){

				pos.mV[VX] = profilep->mProfile[0].mV[VX] * scale.mV[VX];
				pos.mV[VY] = profilep->mProfile[0].mV[VY] * scale.mV[VY];
				pos.mV[VZ] = 0.0f;
				pos = pos * rot;
				pos += pathp -> mPath[s].mPos;

				//global transformation: first scale, then rotate, then trans


				pos.scaleVec(scale_agent);
				pos.rotVec(rot_agent);
				pos += pos_agent;
//				}
				mManipParts[0].push_back(pos);
			}
			//	NK_PROFILE_START_BOTTOM, //Path start
			mManipParts[1].clear();

			pos.set(pathp->mPath[0].mPos);
			pos.scaleVec(scale_agent);
			pos.rotVec(rot_agent);
			pos += pos_agent;
			mManipParts[1].push_back(pos);
			mManipParts[1].push_back(mManipParts[0][0]);
			//	NK_PROFILE_START_TOP, //Path end
			mManipParts[2].clear();

			pos.set(pathp->mPath[sizeS - 1].mPos);
			pos.scaleVec(scale_agent);
			pos.rotVec(rot_agent);
			pos += pos_agent;
			mManipParts[2].push_back(pos);
			mManipParts[2].push_back(mManipParts[0][sizeS - 1]);

			//	NK_PROFILE_END,
			mManipParts[3].clear();
			for (S32 s = 0; s < sizeS; s++){
				LLVector2 scale = pathp->mPath[s].mScale;
				LLQuaternion rot = pathp -> mPath[s]. mRot;

				//for(S32 t = 0; t < 1; t++){

				pos.mV[VX] = profilep->mProfile[sizeTOuter-1].mV[VX] * scale.mV[VX];
				pos.mV[VY] = profilep->mProfile[sizeTOuter-1].mV[VY] * scale.mV[VY];
				pos.mV[VZ] = 0.0f;
				pos = pos * rot;
				pos += pathp -> mPath[s].mPos;

				//global transformation: first scale, then rotate, then trans


				pos.scaleVec(scale_agent);
				pos.rotVec(rot_agent);
				pos += pos_agent;
//				}
				mManipParts[3].push_back(pos);
			}
			//	NK_PROFILE_END_BOTTOM,
			mManipParts[4].clear();

			pos.set(pathp->mPath[0].mPos);
			pos.scaleVec(scale_agent);
			pos.rotVec(rot_agent);
			pos += pos_agent;
			mManipParts[4].push_back(pos);
			mManipParts[4].push_back(mManipParts[3][0]);
			//	NK_PROFILE_END_TOP
			mManipParts[5].clear();

			pos.set(pathp->mPath[sizeS - 1].mPos);
			pos.scaleVec(scale_agent);
			pos.rotVec(rot_agent);
			pos += pos_agent;
			mManipParts[5].push_back(pos);
			mManipParts[5].push_back(mManipParts[3][sizeS - 1]);
		}
	}

	;
}
///generate the paths the manipulators move
void NKManipProfileCut::generateManipulatorPaths()
{

	LLVector3 pos;
	LLBBox bbox;
	if (mvo){
		LLVolume* volumep = mvo->getVolume();
		if (volumep){
			LLPath* pathp = &(volumep->getPath());
			const LLProfile* profilep = &(volumep->getProfile());

			bbox = mvo->getBoundingBoxAgent();
			LLVector3 pos_agent = bbox.getPositionAgent();
			LLQuaternion rot_agent = bbox.getRotation();
			LLVector3 scale_agent = mvo->getScale();

			S32 sizeS = pathp->mPath.size();

//			S32 sizeTOuter = profilep->getTotalOut();
//			sizeTOuter = (sizeTOuter != 0) ? sizeTOuter : sizeT;
			//sizeTOuter = (sizeTOuter) ? sizeTOuter : sizeT;

			switch (mHighlightedPart){
			case NK_PROFILE_START:
			case NK_PROFILE_END:

				//do stuff here
				{
					////////////////////////////
					//first, generate path slice
					LLPath::PathPt pt;
					F32 segmentf = floor(mSelectedPosition * (sizeS - 1));
					S32 segment = (S32) segmentf;
					F32 coeff = mSelectedPosition*(sizeS - 1) - segmentf;

					//lerping the rotation instead of the result means the path will probably be a bit off
					//...
					//What the hek is a nlerp?
					pt.mPos.set(lerp(pathp->mPath[segment].mPos, pathp->mPath[segment + 1].mPos, coeff));
					pt.mScale.set(lerp(pathp->mPath[segment].mScale, pathp->mPath[segment + 1].mScale, coeff));
					pt.mRot.set(nlerp(coeff,pathp->mPath[segment].mRot, pathp->mPath[segment + 1].mRot));
					//texture coord is not needed
					LL_INFOS(NULL)<< "segment = " << segmentf << " coeff= " << coeff << " sizeS= "<< sizeS << LL_ENDL;
					LL_INFOS(NULL)<< pt.mPos << pt.mScale << LL_ENDL;

					//*TODO: if coeff is exactly zero, don't interpolate
					//	might be able to prevent array OOB when mSelectedPosition is exactly 1

					/////////////////////////////
					//next, generate full profile
					LLProfileParams params;
					params.copyParams(volumep->getParams().getProfileParams());
					params.setBegin(0.f);
					params.setEnd(1.f);

					NKProfileUnlock unlock;
					{
						LLProfile profile; //NOTE: this is not profilep

						//detail is llvolume->mDetail
						//*TODO: double it? what about the split?
						profile.generate(params, pathp->isOpen(), volumep->getDetail(), 0);

						//////////////////////////////////////////
						//finally, combine them for the whole path
						mManipPath.clear();
						S32 sizeT = profile.getTotalOut();
						if (sizeT == 0){sizeT = profile.getTotal();}
						if (volumep->mFaceMask & (LL_FACE_PROFILE_END)){
							sizeT -= 1;
						}
						//assuming the profile is closed here

						LLVector3 pos;

		//				LLVector2 scale = pathp->mPath[s].mScale;
		//				LLQuaternion rot = pathp -> mPath[s]. mRot;
						for (S32 t = 0; t < sizeT; t++){

							pos.mV[VX] = profile.mProfile[t].mV[VX] * pt.mScale.mV[VX];
							pos.mV[VY] = profile.mProfile[t].mV[VY] * pt.mScale.mV[VY];
							pos.mV[VZ] = 0.0f;
							pos = pos * pt.mRot;
							pos += pt.mPos;

							//global transformation: first scale, then rotate, then trans

							pos.scaleVec(scale_agent);
							pos.rotVec(rot_agent);
							pos += pos_agent;
			//				}
							mManipPath.push_back(pos);
						}
					}
				}
				//Huzzah, we're finally done!
				break;
			case NK_PROFILE_START_BOTTOM:
			case NK_PROFILE_END_BOTTOM:
			case NK_PROFILE_START_TOP:
			case NK_PROFILE_END_TOP:
			default:
				LL_WARNS("")<< "invalid part" <<LL_ENDL;
			}
		}
	}

	;
}
/// figure out which manipulator is the mouse pointing at
void NKManipProfileCut::updateProximity(S32 x, S32 y)
{
	//no selected object or not a prim - depends on short-circuiting
	//don't even bother clearing the vectors
	if(!mvo || (!mvo->getVolume())){mHighlightedPart = LL_NO_PART; return;}

	//TODO: optimize this - no need to continue if cursor outside bounding box

	//Used to figure out mouse cursor direction. Terrible names, I know.
	LLVector3 selected_segment_1;
	LLVector3 selected_segment_2;

	//selected part (tentative)
	EManipPart part = LL_NO_PART;
	//distance of closest part to mouse cursor
	F32 dist = MANIPULATOR_HOTSPOT_WIDTH * 999;

	//this will slow down framerate a lot...
	S32 numParts = mManipParts.size();
	for(S32 i = 0; i < numParts; i++){
		S32 numSegs = mManipParts[i].size() -1;
		for(S32 j = 0; j < numSegs; j++){
			LLVector3& b1 = mManipParts[i][j];
			LLVector3& b2= mManipParts[i][j+1];
			F32 a_param;
			F32 b_param;

			if(!nearestPointOnLineFromMouse(x,y,b1,b2,a_param,b_param)){
				if (a_param > 0.0f && b_param >= 0.0f && b_param <= 1.0f){ //don't care about intersections behind screen
					//get the point b1+b_param(b2-b1)
					LLVector3 b(b1 + b_param * (b2-b1));
					LLCoordGL c;
					LLViewerCamera::getInstance()->projectPosAgentToScreen(b,c,false);

					//might want to break here to see if the coords match up
					LLVector2 line_pos(c.mX, c.mY);
					LLVector2 mouse_pos(x, y);

					F32 line_to_mouse = dist_vec(line_pos, mouse_pos);
					if (line_to_mouse < MANIPULATOR_HOTSPOT_WIDTH){
						if (line_to_mouse < dist){
							dist = line_to_mouse;

							//part = NK_PROFILE_MIN + i
							//*FIXME: that's a type mismatch, somehow...
							switch (i){
							case 0: part = NK_PROFILE_START; break;
							case 1: part = NK_PROFILE_START_BOTTOM; break;
							case 2: part = NK_PROFILE_START_TOP; break;
							case 3: part = NK_PROFILE_END; break;
							case 4: part = NK_PROFILE_END_BOTTOM; break;
							case 5: part = NK_PROFILE_END_TOP; break;
							default: part = LL_NO_PART;
							}
							mSelectedPosition = ((j+b_param)/numSegs);
							selected_segment_1 = b1;
							selected_segment_2 = b2;
						}
					}
				}
			}
		}
		if (part != LL_NO_PART){ // one is selected


			//still more bad variable names
			LLCoordGL p1;
			LLCoordGL p2;
			LLViewerCamera::getInstance()->projectPosAgentToScreen(selected_segment_1, p1);
			LLViewerCamera::getInstance()->projectPosAgentToScreen(selected_segment_2, p2);
			//calculate cursor slope (= line slope + PI/2)
			//[(0/4-1/8)*PI, (0/4+1/8)*PI] : UI_CURSOR_SIZEWE,
			//[(1/4-1/8)*PI, (1/4+1/8)*PI] : UI_CURSOR_SIZENESW,
			//[(2/4-1/8)*PI, (2/4+1/8)*PI] : UI_CURSOR_SIZENS,
			//[(3/4-1/8)*PI, (3/4+1/8)*PI] : UI_CURSOR_SIZENWSE

			//aka: (line slope + PI/2 + PI/8) / PI * 4 % 4
			//GL coords is the same as trig coords (+y goes up)
			//whose idea is it to have atan(y, x)...
			S32 direction =	llfloor(
				(atan2(p2.mY-p1.mY,p2.mX-p1.mX)
				/ F_PI + (0.5f + 0.125f + 2.0f)) * 4.0f
				) % 4;

			//not a good idea to actually set it here
			//assuming setting the cursor over and over has little performance penalty
			LLWindow* window = gViewerWindow->getWindow();
			switch (direction){
			case 0: window->setCursor(UI_CURSOR_SIZEWE); break;
			case 1: window->setCursor(UI_CURSOR_SIZENESW); break;
			case 2: window->setCursor(UI_CURSOR_SIZENS); break;
			case 3: window->setCursor(UI_CURSOR_SIZENWSE); break;
			default:
				LL_WARNS("NULL")<< "bad cursor direction "<< direction <<LL_ENDL;
			}
		} else {
			gViewerWindow->getWindow()->setCursor(UI_CURSOR_ARROW);
		}
	}


	mHighlightedPart = part;
	;
}
void NKManipProfileCut::drag(S32 x, S32 y, MASK mask)
{
	;
}
void NKManipProfileCut::updateSelection(){
	;
}
