/*
 * NKManipTest.cpp
 *
 *  Created on: 2010-3-31
 *      Author: nekoyasha
 */
#include "llviewerprecompiledheaders.h"

#include <string>

#include "nkmaniptest.h"
	//also includes llviewerobject.h

#include "llgl.h" //use gGL instead of directly calling GL to buffer up calls
#include "llselectmgr.h"
#include "llbbox.h" //already included in llselectmgr.h
#include "llagent.h"
#include "llvolume.h"
#include "llviewerwindow.h"

NKManipTest::NKManipTest()
: LLManip (std::string("dummy"), NULL)
{	// TODO Auto-generated constructor stub
}

NKManipTest::~NKManipTest() {
	// TODO Auto-generated destructor stub
}

void NKManipTest::handleSelect(){
	//*TODO separate the "new object" logic from here
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	//*TODO this might not be the most descriptive name here...
	mvo = mObjectSelection -> getFirstObject();
}

void NKManipTest::handleDeselect(){
	//*TODO free stoff so there is no mem leak
}

BOOL NKManipTest::handleMouseDown(S32 x, S32 y, MASK mask)
{
	//*TODO test if in proxymity of manipulators
	gViewerWindow->pickAsync(x, y, mask, pickCallback);
	return TRUE;
}
///Pick callback.
///
///mostly copied from LLToolIndividual::pickCallback() ...
void NKManipTest::pickCallback(const LLPickInfo& pick_info)
{
	LLViewerObject* obj = pick_info.getObject();
	LLSelectMgr::getInstance()->deselectAll();
	if(obj)
	{
		//*TODO separate the "new object" logic from here
		LLSelectMgr::getInstance()->selectObjectOnly(obj);
		//*HACK
		NKManipTest::getInstance()->mvo=obj;

#if 1
		LLVolume* volumep = obj->getVolume();
		if (volumep){
			LLBBox bbox;
			LLPath* pathp = &(volumep->getPath());
			const LLProfile* profilep = &(volumep->getProfile());

			bbox = obj->getBoundingBoxAgent();
			LLVector3 pos = bbox.getPositionAgent();
			LLQuaternion rot = bbox.getRotation();
			F32 roll; F32 pitch; F32 yaw;
			rot.getEulerAngles(&roll, &pitch, &yaw);
			LLVector3 scale = obj->getScale();
			
			char buf[1024];
			LL_INFOS(NULL) << "Selected object data" <<LL_ENDL;
			sprintf(buf, "Position: %.3f, %.3f, %.3f", pos.mV[VX], pos.mV[VY], pos.mV[VZ]);
			LL_INFOS(NULL) << buf <<LL_ENDL;
			sprintf(buf, "rotation: %.3f, %.3f, %.3f",  roll ,pitch , yaw);
			LL_INFOS(NULL) << buf <<LL_ENDL;
			sprintf(buf, "scale: %.3f, %.3f, %.3f", scale.mV[VX], scale.mV[VY], scale.mV[VZ]);
			LL_INFOS(NULL) << buf <<LL_ENDL;

			S32 sizeS = pathp->mPath.size();
			LL_INFOS(NULL) << "Path data"<<LL_ENDL;
			for (S32 s = 0; s < sizeS; s++)
			{
				LLPath::PathPt& v = pathp->mPath[s];
				F32 roll; F32 pitch; F32 yaw; v.mRot.getEulerAngles(&roll, &pitch, &yaw);
				sprintf(buf, "%d: %.3f, %.3f, %.3f | %.3f, %.3f, %.3f | %.3f, %.3f| %.3f", s, v.mPos.mV[VX] , v.mPos.mV[VY] ,v.mPos.mV[VZ], roll , pitch , yaw , v.mScale.mV[VX] , v.mScale.mV[VY] ,  v.mTexT);
				LL_INFOS(NULL) << buf << LL_ENDL;
				
			}
			S32 sizeT = profilep->mProfile.size();
			LL_INFOS(NULL) << "Profile data" <<LL_ENDL;
			for (S32 t = 0; t < sizeT; t++)
			{
				const LLVector3& v = profilep->mProfile[t];
				sprintf(buf, "%d: %.3f, %.3f, %.3f", t, v.mV[VX], v.mV[VY], v.mV[VZ]);
				LL_INFOS(NULL) << buf <<LL_ENDL;
			}
		}
#endif
		
	}

	
}

void NKManipTest::draw(){
	//gsomething -> setup2DRender();
	;
}
void NKManipTest::render(){
	LLBBox bbox;
	LLGLDepthTest gls_depth(GL_FALSE);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	if (mObjectSelection->getSelectType() == SELECT_TYPE_HUD)
	{
		F32 zoom = gAgent.mHUDCurZoom;
		glScalef(zoom, zoom, zoom);
	}
	gGL.getTexUnit(0)->unbind(LLTexUnit::TT_TEXTURE);
	if (mvo)
	{
		LLVolume* volumep = mvo->getVolume();
		if (volumep){
			LLPath* pathp = &(volumep->getPath());
			const LLProfile* profilep = &(volumep->getProfile());

			bbox = mvo->getBoundingBoxAgent();
			LLVector3 pos = bbox.getPositionAgent();
			LLQuaternion rot = bbox.getRotation();
			LLVector3 scale = mvo->getScale();
			F32 angle_radians, x, y, z;

			glTranslatef(pos.mV[VX], pos.mV[VY], pos.mV[VZ]);

			rot.getAngleAxis(&angle_radians, &x, &y, &z);
			glRotatef(angle_radians * RAD_TO_DEG, x, y, z);

			glScalef(scale.mV[VX], scale.mV[VY], scale.mV[VZ]);
			
			glColor4f(0.2f, 0.5f, 0.8f, 0.8f);
			glLineWidth(1.8f);

			S32 sizeS = pathp->mPath.size();
			glBegin(GL_LINE_STRIP);
			for (S32 s = 0; s < sizeS; s++)
			{
				LLVector3& v = pathp->mPath[s].mPos;
				glVertex3f(v.mV[VX], v.mV[VY], v.mV[VZ]);
			}
			glEnd();

			glColor4f(0.8f, 0.5f, 0.2f, 0.8f);
			S32 sizeT = profilep->mProfile.size();
			glBegin(GL_LINE_STRIP);
			for (S32 t = 0; t < sizeT; t++)
			{
				const LLVector3& v = profilep->mProfile[t];
				//not sure what the Z value is used for...
				glVertex2f(v.mV[VX], v.mV[VY]);
			}
			glEnd();
		}
	}
	glPopMatrix();
	;
}
