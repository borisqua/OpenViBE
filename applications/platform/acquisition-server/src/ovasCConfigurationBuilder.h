#ifndef __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__
#define __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__

#include "ovas_base.h"

#include <gtk/gtk.h>

#include <string>
#include <map>

namespace OpenViBEAcquisitionServer
{
	class IHeader;

	class CConfigurationBuilder
	{
	public:
		typedef enum {
			Error_NoError        = 0,
			Error_UserCancelled  = 1,
			Error_Unknown        = 2
		} EErrorCode;

		CConfigurationBuilder(const char* sGtkBuilderFileName);
		virtual ~CConfigurationBuilder(void);

		virtual OpenViBE::boolean configure(
			OpenViBEAcquisitionServer::IHeader& rHeader);

		virtual void buttonChangeChannelNamesCB(void);
		virtual void buttonApplyChannelNameCB(void);
		virtual void buttonRemoveChannelNameCB(void);
		virtual void treeviewApplyChannelNameCB(void);

	protected:

		virtual OpenViBE::boolean preConfigure(void);
		virtual OpenViBE::boolean doConfigure(EErrorCode& result);
		virtual OpenViBE::boolean postConfigure(void);

	private:

		CConfigurationBuilder(void);

	protected:

		OpenViBE::boolean m_bApplyConfiguration;

		::GtkBuilder* m_pBuilderConfigureInterface;
		::GtkBuilder* m_pBuilderConfigureChannelInterface;

		::GtkWidget* m_pDialog;

		::GtkWidget* m_pIdentifier;
		::GtkWidget* m_pAge;
		::GtkWidget* m_pNumberOfChannels;
		::GtkWidget* m_pSamplingFrequency;
		::GtkWidget* m_pGender;
		::GtkWidget* m_pImpedanceCheck;

		::GtkListStore* m_pElectrodeNameListStore;
		::GtkListStore* m_pChannelNameListStore;

		::GtkWidget* m_pElectrodeNameTreeView;
		::GtkWidget* m_pChannelNameTreeView;

		std::map < OpenViBE::uint32, std::string > m_vChannelName;
		std::string m_sGtkBuilderFileName;
		std::string m_sElectrodeFileName;
		std::string m_sGtkBuilderChannelsFileName;
		IHeader* m_pHeader;
	};
};

#endif // __OpenViBE_AcquisitionServer_CConfigurationBuilder_H__
