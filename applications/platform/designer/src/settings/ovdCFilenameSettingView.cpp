
#include "ovdCFilenameSettingView.h"
#include "../ovd_base.h"

#include <iostream>
#include <cstring>

using namespace OpenViBE;
using namespace OpenViBEDesigner;
using namespace OpenViBEDesigner::Setting;

static void on_button_setting_filename_browse_pressed(::GtkButton* pButton, gpointer pUserData)
{
	static_cast< CFilenameSettingView * >(pUserData)->browse();
}

static void on_change(::GtkEntry *entry, gpointer pUserData)
{
	static_cast<CFilenameSettingView *>(pUserData)->onChange();
}


CFilenameSettingView::CFilenameSettingView(OpenViBE::Kernel::IBox &rBox, OpenViBE::uint32 ui32Index, CString &rBuilderName, const Kernel::IKernelContext &rKernelContext):
	CAbstractSettingView(rBox, ui32Index, rBuilderName, "settings_collection-hbox_setting_filename"), m_rKernelContext(rKernelContext), m_bOnValueSetting(false)
{
	::GtkWidget* l_pSettingWidget = this->getEntryFieldWidget();

	std::vector< ::GtkWidget* > l_vWidget;
	extractWidget(l_pSettingWidget, l_vWidget);
	m_pEntry = GTK_ENTRY(l_vWidget[0]);

	g_signal_connect(G_OBJECT(m_pEntry), "changed", G_CALLBACK(on_change), this);
	g_signal_connect(G_OBJECT(l_vWidget[1]), "clicked", G_CALLBACK(on_button_setting_filename_browse_pressed), this);


	initializeValue();
}


void CFilenameSettingView::getValue(OpenViBE::CString &rValue) const
{
	rValue = CString(gtk_entry_get_text(m_pEntry));
}


void CFilenameSettingView::setValue(const OpenViBE::CString &rValue)
{
	m_bOnValueSetting = true;
	gtk_entry_set_text(m_pEntry, rValue);
	m_bOnValueSetting =false;
}

void CFilenameSettingView::browse()
{
	::GtkWidget* l_pWidgetDialogOpen=gtk_file_chooser_dialog_new(
		"Select file to open...",
		NULL,
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
		NULL);

	CString l_sInitialFileName=m_rKernelContext.getConfigurationManager().expand(gtk_entry_get_text(m_pEntry));
	if(g_path_is_absolute(l_sInitialFileName.toASCIIString()))
	{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sInitialFileName.toASCIIString());
	}
	else
	{
		char* l_sFullPath=g_build_filename(g_get_current_dir(), l_sInitialFileName.toASCIIString(), NULL);
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), l_sFullPath);
		g_free(l_sFullPath);
	}

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(l_pWidgetDialogOpen), false);

	if(gtk_dialog_run(GTK_DIALOG(l_pWidgetDialogOpen))==GTK_RESPONSE_ACCEPT)
	{
		char* l_sFileName=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(l_pWidgetDialogOpen));
		char* l_pBackslash = NULL;
		while((l_pBackslash = ::strchr(l_sFileName, '\\'))!=NULL)
		{
			*l_pBackslash = '/';
		}
		getBox().setSettingValue(getSettingIndex(), l_sFileName);
		g_free(l_sFileName);
	}
	gtk_widget_destroy(l_pWidgetDialogOpen);
}

void CFilenameSettingView::onChange()
{
	if(!m_bOnValueSetting)
	{
		const gchar* l_sValue = gtk_entry_get_text(m_pEntry);
		getBox().setSettingValue(getSettingIndex(), l_sValue);
	}
}
