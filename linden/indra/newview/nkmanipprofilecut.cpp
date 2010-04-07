/*
 * NKManipTest.cpp
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

const F32 MANIPULATOR_WIDTH = 1.8f;
const F32 SELECTED_MANIPULATOR_WIDTH = 3.6f;
const F32 MANIPULATOR_HOTSPOT_WIDTH = 3.0f;
const F32 MANIPULATOR_SCALE_HALF_LIFE = 0.07f;
const S32 NUM_MANIPULATORS = 6; //NK_PROFILE_END_TOP - NK_PROFILE_BEGIN + 1

NKManipProfileCut::NKManipProfileCut()
: LLManip (std::string("dummy"), NULL)
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
		//didn't hit manipulators, select objects instead
		//bad idea to pick on mouse DOWN though
	//if there is an active manipulator, enter drag
	//setMouseCapture(TRUE);
	//*TODO test if in proxymity of manipulators
	gViewerWindow->pickAsync(x, y, mask, pickCallback);
	return TRUE;

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
	updateProximity(x, y);
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
void NKManipProfileCut::render(){
	//are we in a drag?

	//start by polling the selection manager.
	//*TODO: we really should be checking for LLVolume / Pcode here
	LLViewerObject* VOp = LLSelectMgr::getInstance()->getEditSelection()-> getFirstObject();
	if (VOp != mvo){
		mvo=VOp;
		generateManipulators();

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
		//*TODO: 2-pass rendering?
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

				pos.mV[VX] = profilep->mProfile[sizeTOuter].mV[VX] * scale.mV[VX];
				pos.mV[VY] = profilep->mProfile[sizeTOuter].mV[VY] * scale.mV[VY];
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
			S32 sizeT = profilep->mProfile.size();
			S32 sizeTOuter = profilep->getTotalOut();

			switch (mHighlightedPart){
			case NK_PROFILE_START:
			case NK_PROFILE_END:
				//do stuff here
				break;
			default:
				LL_WARNS("")<< "invalid part" <<LL_ENDL;
			}
		}
	}

	;
}
void NKManipProfileCut::updateProximity(S32 x, S32 y)
{
	if(!mvo){return;}

	//TODO: optimize this - no need to project if outside bbox of bbox

	EManipPart part = LL_NO_PART;
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
							switch (i){
							case 0: part = NK_PROFILE_START; break;
							case 1: part = NK_PROFILE_START_BOTTOM; break;
							case 2: part = NK_PROFILE_START_TOP; break;
							case 3: part = NK_PROFILE_END; break;
							case 4: part = NK_PROFILE_END_BOTTOM; break;
							case 5: part = NK_PROFILE_END_TOP; break;
							default: part = LL_NO_PART;
							}
							mSelectedPosition = (j/numSegs) + b_param;
						}
					}
				}
			}
		}
	}
	mHighlightedPart = part;
	;
}
void NKManipProfileCut::drag(S32 x, S32 y)
{
	;
}
void NKManipProfileCut::updateSelection(){
	;
}
