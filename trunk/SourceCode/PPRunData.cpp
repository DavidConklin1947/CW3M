// PPRunData.cpp : implementation file
//

#include "stdafx.h"
#include "Envision.h"
#include "resource.h"
#include "PPRunData.h"


// PPRunData dialog

IMPLEMENT_DYNAMIC(PPRunData, CPropertyPage)

PPRunData::PPRunData()
	: CPropertyPage(PPRunData::IDD)
   , m_collectActorData(FALSE)
   , m_collectLandscapeScoreData(FALSE)
   {
}

PPRunData::~PPRunData()
{
}

void PPRunData::DoDataExchange(CDataExchange* pDX)
{
CDialog::DoDataExchange(pDX);
DDX_Check(pDX, IDC_COLLECTACTORDATA, m_collectActorData);
DDX_Check(pDX, IDC_COLLECTLANDSCAPESCOREDATA, m_collectLandscapeScoreData);
}


BEGIN_MESSAGE_MAP(PPRunData, CPropertyPage)
END_MESSAGE_MAP()


// PPRunData message handlers
