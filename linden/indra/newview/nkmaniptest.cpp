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

NKManipTest::NKManipTest()
: LLManip (std::string("dummy"), NULL)
{	// TODO Auto-generated constructor stub
}

NKManipTest::~NKManipTest() {
	// TODO Auto-generated destructor stub
}

void NKManipTest::handleSelect(){
	mObjectSelection = LLSelectMgr::getInstance()->getEditSelection();
	//*TODO this might not be the most descriptive name here...
	mvo = mObjectSelection -> getFirstObject();
}

void NKManipTest::handleDeselect(){
	//*TODO free stoff so there is no mem leak
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
