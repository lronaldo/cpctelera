/*
//
// Copyright (C) 2006, 2007, 2008, 2009, 2010, 2011 Jean-François DEL NERO
//
// This file is part of HxCFloppyEmulator.
//
// HxCFloppyEmulator may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// HxCFloppyEmulator is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// HxCFloppyEmulator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with HxCFloppyEmulator; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/
#include "../common/plugins/adf_loader/adf_loader.h"
#include "../common/plugins/adz_loader/adz_loader.h"
#include "../common/plugins/dms_loader/dms_loader.h"
#include "../common/plugins/msa_loader/msa_loader.h"
#include "../common/plugins/amigadosfs_loader/amigadosfs_loader.h"
#ifdef IPF_SUPPORT 
	#include "../common/plugins/ipf_loader/ipf_loader.h"
#endif
#include "../common/plugins/st_loader/st_loader.h"
#include "../common/plugins/stx_loader/stx_loader.h"
#include "../common/plugins/cpcdsk_loader/cpcdsk_loader.h"
#include "../common/plugins/img_loader/img_loader.h"
#include "../common/plugins/copyqm_loader/copyqm_loader.h"
#include "../common/plugins/oricdsk_loader/oricdsk_loader.h"
#include "../common/plugins/mfm_loader/mfm_loader.h"
#include "../common/plugins/msx_loader/msx_loader.h"
#include "../common/plugins/fat12floppy_loader/fat12floppy_loader.h"
#include "../common/plugins/smc_loader/snes_smc_loader.h"
#include "../common/plugins/hfe_loader/hfe_loader.h"
#include "../common/plugins/imd_loader/imd_loader.h"
#include "../common/plugins/afi_loader/afi_loader.h"
#include "../common/plugins/d64_loader/d64_loader.h"
#include "../common/plugins/d81_loader/d81_loader.h"
#include "../common/plugins/trd_loader/trd_loader.h"
#include "../common/plugins/scl_loader/scl_loader.h"
#include "../common/plugins/sap_loader/sap_loader.h"
#include "../common/plugins/jv1_loader/jv1_loader.h"
#include "../common/plugins/jv3_loader/jv3_loader.h"
#include "../common/plugins/vtr_loader/vtr_loader.h"
#include "../common/plugins/d88_loader/d88_loader.h"
#include "../common/plugins/hdm_loader/hdm_loader.h"
#include "../common/plugins/ti99pc99_loader/ti99pc99_loader.h"
#include "../common/plugins/ti99v9t9_loader/ti99v9t9_loader.h"
#include "../common/plugins/apridisk_loader/apridisk_loader.h"
#include "../common/plugins/ede_loader/ede_loader.h"
#include "../common/plugins/fd_loader/fd_loader.h"
#include "../common/plugins/vdk_loader/vdk_loader.h"
#include "../common/plugins/dpx_loader/dpx_loader.h"
#include "../common/plugins/ensoniq_mirage_loader/ensoniq_mirage_loader.h"
#include "../common/plugins/emax_loader/emax_loader.h"
#include "../common/plugins/mgt_loader/mgt_loader.h"
#include "../common/plugins/sad_loader/sad_loader.h"
#include "../common/plugins/stt_loader/stt_loader.h"
#include "../common/plugins/prophet_loader/prophet_loader.h"
#include "../common/plugins/teledisk_loader/teledisk_loader.h"
#include "../common/plugins/emuii_raw_loader/emuii_raw_loader.h"
#include "../common/plugins/emuii_loader/emuii_loader.h"
#include "../common/plugins/emui_raw_loader/emui_raw_loader.h"
#include "../common/plugins/jvc_loader/jvc_loader.h"
#include "../common/plugins/dim_loader/dim_loader.h"
#include "../common/plugins/dmk_loader/dmk_loader.h"
#include "../common/plugins/acornadf_loader/acornadf_loader.h"
#include "../common/plugins/vegasdsk_loader/vegasdsk_loader.h"
#include "../common/plugins/camputerslynxldf_loader/camputerslynxldf_loader.h"
#include "../common/plugins/extadf_loader/extadf_loader.h"
#include "../common/plugins/oldextadf_loader/oldextadf_loader.h"
#include "../common/plugins/fdi_loader/fdi_loader.h"
#include "../common/plugins/adl_loader/adl_loader.h"
#include "../common/plugins/ssd_dsd_loader/ssd_dsd_loader.h"
#include "../common/plugins/krz_loader/krz_loader.h"
#include "../common/plugins/w30_loader/w30_loader.h"
#include "../common/plugins/fei_loader/fei_loader.h"
#include "../common/plugins/svd_loader/svd_loader.h"
#include "../common/plugins/imz_loader/imz_loader.h"
#include "../common/plugins/gkh_loader/gkh_loader.h"

const plugins_ptr staticplugins[]=
{
	{(ISVALIDDISKFILE)DMS_libIsValidDiskFile,(LOADDISKFILE)DMS_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)ADZ_libIsValidDiskFile,(LOADDISKFILE)ADZ_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EXTADF_libIsValidDiskFile,(LOADDISKFILE)EXTADF_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)OLDEXTADF_libIsValidDiskFile,(LOADDISKFILE)OLDEXTADF_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)FDI_libIsValidDiskFile,(LOADDISKFILE)FDI_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)ADF_libIsValidDiskFile,(LOADDISKFILE)ADF_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)ACORNADF_libIsValidDiskFile,(LOADDISKFILE)ACORNADF_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)CPCDSK_libIsValidDiskFile,(LOADDISKFILE)CPCDSK_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)DIM_libIsValidDiskFile,(LOADDISKFILE)DIM_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)STX_libIsValidDiskFile,(LOADDISKFILE)STX_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)STT_libIsValidDiskFile,(LOADDISKFILE)STT_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)CopyQm_libIsValidDiskFile,(LOADDISKFILE)CopyQm_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)TeleDisk_libIsValidDiskFile,(LOADDISKFILE)TeleDisk_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)MSA_libIsValidDiskFile,(LOADDISKFILE)MSA_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)IMZ_libIsValidDiskFile,(LOADDISKFILE)IMZ_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)MFM_libIsValidDiskFile,(LOADDISKFILE)MFM_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)OricDSK_libIsValidDiskFile,(LOADDISKFILE)OricDSK_libLoad_DiskFile,0,0},	
	{(ISVALIDDISKFILE)ST_libIsValidDiskFile,(LOADDISKFILE)ST_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)W30_libIsValidDiskFile,(LOADDISKFILE)W30_libLoad_DiskFile,0,0},
	#ifdef IPF_SUPPORT
	{(ISVALIDDISKFILE)IPF_libIsValidDiskFile,(LOADDISKFILE)IPF_libLoad_DiskFile,0,0},
	#endif
	{(ISVALIDDISKFILE)TI99V9T9_libIsValidDiskFile,(LOADDISKFILE)TI99V9T9_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)AMIGADOSFSDK_libIsValidDiskFile,(LOADDISKFILE)AMIGADOSFSDK_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)Prophet_libIsValidDiskFile,(LOADDISKFILE)Prophet_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)IMG_libIsValidDiskFile,(LOADDISKFILE)IMG_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)MSX_libIsValidDiskFile,(LOADDISKFILE)MSX_libLoad_DiskFile,0,0},	
	{(ISVALIDDISKFILE)FAT12FLOPPY_libIsValidDiskFile,(LOADDISKFILE)FAT12FLOPPY_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)HFE_libIsValidDiskFile,(LOADDISKFILE)HFE_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)VTR_libIsValidDiskFile,(LOADDISKFILE)VTR_libLoad_DiskFile,0,0},	
	{(ISVALIDDISKFILE)IMD_libIsValidDiskFile,(LOADDISKFILE)IMD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)AFI_libIsValidDiskFile,(LOADDISKFILE)AFI_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)D64_libIsValidDiskFile,(LOADDISKFILE)D64_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)D81_libIsValidDiskFile,(LOADDISKFILE)D81_libLoad_DiskFile,0,0},	
	{(ISVALIDDISKFILE)TRD_libIsValidDiskFile,(LOADDISKFILE)TRD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)SCL_libIsValidDiskFile,(LOADDISKFILE)SCL_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)SAP_libIsValidDiskFile,(LOADDISKFILE)SAP_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)JV1_libIsValidDiskFile,(LOADDISKFILE)JV1_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)JV3_libIsValidDiskFile,(LOADDISKFILE)JV3_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)JVC_libIsValidDiskFile,(LOADDISKFILE)JVC_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)SVD_libIsValidDiskFile,(LOADDISKFILE)SVD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)D88_libIsValidDiskFile,(LOADDISKFILE)D88_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)HDM_libIsValidDiskFile,(LOADDISKFILE)HDM_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)snes_smc_libIsValidDiskFile,(LOADDISKFILE)snes_smc_libLoad_DiskFile,0,0},
	//{(ISVALIDDISKFILE)KRZ_libIsValidDiskFile,(LOADDISKFILE)KRZ_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)VEGASDSK_libIsValidDiskFile,(LOADDISKFILE)VEGASDSK_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)DMK_libIsValidDiskFile,(LOADDISKFILE)DMK_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)TI99PC99_libIsValidDiskFile,(LOADDISKFILE)TI99PC99_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)ApriDisk_libIsValidDiskFile,(LOADDISKFILE)ApriDisk_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EDE_libIsValidDiskFile,(LOADDISKFILE)EDE_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)GKH_libIsValidDiskFile,(LOADDISKFILE)GKH_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)FD_libIsValidDiskFile,(LOADDISKFILE)FD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)VDK_libIsValidDiskFile,(LOADDISKFILE)VDK_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)DPX_libIsValidDiskFile,(LOADDISKFILE)DPX_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)Ensoniq_mirage_libIsValidDiskFile,(LOADDISKFILE)Ensoniq_mirage_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EMAX_libIsValidDiskFile,(LOADDISKFILE)EMAX_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)MGT_libIsValidDiskFile,(LOADDISKFILE)MGT_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)SAD_libIsValidDiskFile,(LOADDISKFILE)SAD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EMUII_RAW_libIsValidDiskFile,(LOADDISKFILE)EMUII_RAW_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EMUII_libIsValidDiskFile,(LOADDISKFILE)EMUII_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)EMUI_RAW_libIsValidDiskFile,(LOADDISKFILE)EMUI_RAW_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)CAMPUTERSLYNX_libIsValidDiskFile,(LOADDISKFILE)CAMPUTERSLYNX_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)ADL_libIsValidDiskFile,(LOADDISKFILE)ADL_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)DSD_libIsValidDiskFile,(LOADDISKFILE)DSD_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)FEI_libIsValidDiskFile,(LOADDISKFILE)FEI_libLoad_DiskFile,0,0},
	{(ISVALIDDISKFILE)-1,(LOADDISKFILE)-1,(WRITEDISKFILE)-1,(GETPLUGININFOS)-1}
};
