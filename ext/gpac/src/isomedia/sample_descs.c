/*
 *			GPAC - Multimedia Framework C SDK
 *
 *			Copyright (c) Jean Le Feuvre 2000-2005 
 *					All rights reserved
 *
 *  This file is part of GPAC / ISO Media File Format sub-project
 *
 *  GPAC is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *   
 *  GPAC is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *   
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. 
 *
 */

#include <gpac/internal/isomedia_dev.h>

void gf_isom_video_sample_entry_init(GF_VisualSampleEntryBox *ent)
{
	ent->horiz_res = ent->vert_res = 0x00480000;
	ent->frames_per_sample = 1;
	ent->bit_depth = 0x18;
	ent->color_table_index = -1;
}

GF_Err gf_isom_video_sample_entry_read(GF_VisualSampleEntryBox *ptr, GF_BitStream *bs)
{
	if (ptr->size < 78) return GF_ISOM_INVALID_FILE;
	ptr->size -= 78;
	gf_bs_read_data(bs, ptr->reserved, 6);
	ptr->dataReferenceIndex = gf_bs_read_u16(bs);
	ptr->version = gf_bs_read_u16(bs);
	ptr->revision = gf_bs_read_u16(bs);
	ptr->vendor = gf_bs_read_u32(bs);
	ptr->temporal_quality  = gf_bs_read_u32(bs);
	ptr->spacial_quality = gf_bs_read_u32(bs);
	ptr->Width = gf_bs_read_u16(bs);
	ptr->Height = gf_bs_read_u16(bs);
	ptr->horiz_res = gf_bs_read_u32(bs);
	ptr->vert_res = gf_bs_read_u32(bs);
	ptr->entry_data_size = gf_bs_read_u32(bs);
	ptr->frames_per_sample = gf_bs_read_u16(bs);
	gf_bs_read_data(bs, ptr->compressor_name, 32);
	ptr->compressor_name[32] = 0;
	ptr->bit_depth = gf_bs_read_u16(bs);
	ptr->color_table_index = gf_bs_read_u16(bs);
	return GF_OK;
}

#ifndef GPAC_READ_ONLY
void gf_isom_video_sample_entry_write(GF_VisualSampleEntryBox *ptr, GF_BitStream *bs)
{

	gf_bs_write_data(bs, ptr->reserved, 6);
	gf_bs_write_u16(bs, ptr->dataReferenceIndex);
	
	gf_bs_write_u16(bs, ptr->version);
	gf_bs_write_u16(bs, ptr->revision);
	gf_bs_write_u32(bs, ptr->vendor);
	gf_bs_write_u32(bs, ptr->temporal_quality);
	gf_bs_write_u32(bs, ptr->spacial_quality);
	gf_bs_write_u16(bs, ptr->Width);
	gf_bs_write_u16(bs, ptr->Height);
	gf_bs_write_u32(bs, ptr->horiz_res);
	gf_bs_write_u32(bs, ptr->vert_res);
	gf_bs_write_u32(bs, ptr->entry_data_size);
	gf_bs_write_u16(bs, ptr->frames_per_sample);
	gf_bs_write_data(bs, ptr->compressor_name, 32);
	gf_bs_write_u16(bs, ptr->bit_depth);
	gf_bs_write_u16(bs, ptr->color_table_index);
}

void gf_isom_video_sample_entry_size(GF_VisualSampleEntryBox *ent)
{
	ent->size += 78;
}


#endif



void gf_isom_audio_sample_entry_init(GF_AudioSampleEntryBox *ptr)
{
	ptr->channel_count = 2;
	ptr->bitspersample = 16;
}

GF_Err gf_isom_audio_sample_entry_read(GF_AudioSampleEntryBox *ptr, GF_BitStream *bs)
{
	if (ptr->size<28) return GF_ISOM_INVALID_FILE;
	ptr->size -= 28;
	gf_bs_read_data(bs, ptr->reserved, 6);
	ptr->dataReferenceIndex = gf_bs_read_u16(bs);
	ptr->version = gf_bs_read_u16(bs);
	ptr->revision = gf_bs_read_u16(bs);
	ptr->vendor = gf_bs_read_u32(bs);
	ptr->channel_count = gf_bs_read_u16(bs);
	ptr->bitspersample = gf_bs_read_u16(bs);
	ptr->compression_id = gf_bs_read_u16(bs);
	ptr->packet_size = gf_bs_read_u16(bs);
	ptr->samplerate_hi = gf_bs_read_u16(bs);
	ptr->samplerate_lo = gf_bs_read_u16(bs);

	if (ptr->version==1) {
		if (ptr->size<16) return GF_ISOM_INVALID_FILE;
		gf_bs_skip_bytes(bs, 16);
		ptr->size-=16;
	} else if (ptr->version==2) {
		if (ptr->size<72) return GF_ISOM_INVALID_FILE;
		gf_bs_skip_bytes(bs, 72);
		ptr->size-=72;
	}
	return GF_OK;
}

#ifndef GPAC_READ_ONLY

void gf_isom_audio_sample_entry_write(GF_AudioSampleEntryBox *ptr, GF_BitStream *bs)
{
	gf_bs_write_data(bs, ptr->reserved, 6);
	gf_bs_write_u16(bs, ptr->dataReferenceIndex);

	gf_bs_write_u16(bs, ptr->version);
	gf_bs_write_u16(bs, ptr->revision);
	gf_bs_write_u32(bs, ptr->vendor);
	gf_bs_write_u16(bs, ptr->channel_count);
	gf_bs_write_u16(bs, ptr->bitspersample);
	gf_bs_write_u16(bs, ptr->compression_id);
	gf_bs_write_u16(bs, ptr->packet_size);
	gf_bs_write_u16(bs, ptr->samplerate_hi);
	gf_bs_write_u16(bs, ptr->samplerate_lo);
}

void gf_isom_audio_sample_entry_size(GF_AudioSampleEntryBox *ptr)
{
	ptr->size += 28;
}


#endif	//GPAC_READ_ONLY



GF_EXPORT
GF_3GPConfig *gf_isom_3gp_config_get(GF_ISOFile *the_file, u32 trackNumber, u32 StreamDescriptionIndex)
{
	GF_3GPConfig *config, *res;
	GF_TrackBox *trak;
	GF_SampleEntryBox *entry;
	trak = gf_isom_get_track_from_file(the_file, trackNumber);
	if (!trak || !StreamDescriptionIndex) return NULL;

	config = NULL;
	entry = (GF_SampleEntryBox *)gf_list_get(trak->Media->information->sampleTable->SampleDescription->boxList, StreamDescriptionIndex-1);
	if (!entry) return NULL;
	switch (entry->type) {
	case GF_ISOM_SUBTYPE_3GP_AMR:
	case GF_ISOM_SUBTYPE_3GP_AMR_WB:
	case GF_ISOM_SUBTYPE_3GP_EVRC:
	case GF_ISOM_SUBTYPE_3GP_QCELP:
	case GF_ISOM_SUBTYPE_3GP_SMV:
		if (! ((GF_3GPPAudioSampleEntryBox*)entry)->info) return NULL;
		config = & ((GF_3GPPAudioSampleEntryBox*)entry)->info->cfg;
		break;
	case GF_ISOM_SUBTYPE_3GP_H263:
		if (! ((GF_3GPPVisualSampleEntryBox*)entry)->info) return NULL;
		config = & ((GF_3GPPVisualSampleEntryBox*)entry)->info->cfg;
		break;
	default:
		return NULL;
	}
	if (!config) return NULL;

	res = (GF_3GPConfig*)malloc(sizeof(GF_3GPConfig));
	memcpy(res, config, sizeof(GF_3GPConfig));
	return res;
}

#ifndef GPAC_READ_ONLY

GF_EXPORT
GF_Err gf_isom_3gp_config_new(GF_ISOFile *the_file, u32 trackNumber, GF_3GPConfig *cfg, char *URLname, char *URNname, u32 *outDescriptionIndex)
{
	GF_TrackBox *trak;
	GF_Err e;
	u32 dataRefIndex;
	GF_TrackReferenceTypeBox *dpnd;
	GF_TrackReferenceBox *tref;
	u32 cfg_type;

	e = CanAccessMovie(the_file, GF_ISOM_OPEN_WRITE);
	if (e) return e;
	
	trak = gf_isom_get_track_from_file(the_file, trackNumber);
	if (!trak || !trak->Media || !cfg) return GF_BAD_PARAM;

	switch (cfg->type) {
	case GF_ISOM_SUBTYPE_3GP_AMR:
	case GF_ISOM_SUBTYPE_3GP_AMR_WB:
		if (trak->Media->handler->handlerType!=GF_ISOM_MEDIA_AUDIO) return GF_BAD_PARAM;
		cfg_type = GF_ISOM_BOX_TYPE_DAMR;
		break;
	case GF_ISOM_SUBTYPE_3GP_EVRC: 
		if (trak->Media->handler->handlerType!=GF_ISOM_MEDIA_AUDIO) return GF_BAD_PARAM;
		cfg_type = GF_ISOM_BOX_TYPE_DEVC;
		break;
	case GF_ISOM_SUBTYPE_3GP_QCELP:
		if (trak->Media->handler->handlerType!=GF_ISOM_MEDIA_AUDIO) return GF_BAD_PARAM;
		cfg_type = GF_ISOM_BOX_TYPE_DQCP;
		break;
	case GF_ISOM_SUBTYPE_3GP_SMV:
		if (trak->Media->handler->handlerType!=GF_ISOM_MEDIA_AUDIO) return GF_BAD_PARAM;
		cfg_type = GF_ISOM_BOX_TYPE_DSMV;
		break;
	case GF_ISOM_SUBTYPE_3GP_H263:
		if (trak->Media->handler->handlerType!=GF_ISOM_MEDIA_VISUAL) return GF_BAD_PARAM;
		cfg_type = GF_ISOM_BOX_TYPE_D263;
		break;
	case 0: return GF_BAD_PARAM;
	default:
		return GF_NOT_SUPPORTED;
	}
	dpnd = NULL;
	tref = NULL;


	//get or create the data ref
	e = Media_FindDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
	if (e) return e;
	if (!dataRefIndex) {
		e = Media_CreateDataRef(trak->Media->information->dataInformation->dref, URLname, URNname, &dataRefIndex);
		if (e) return e;
	}
	trak->Media->mediaHeader->modificationTime = gf_isom_get_mp4time();

	switch (cfg->type) {
	case GF_ISOM_SUBTYPE_3GP_AMR:
	case GF_ISOM_SUBTYPE_3GP_AMR_WB:
	case GF_ISOM_SUBTYPE_3GP_EVRC:
	case GF_ISOM_SUBTYPE_3GP_QCELP:
	case GF_ISOM_SUBTYPE_3GP_SMV:
	{
		GF_3GPPAudioSampleEntryBox *entry = (GF_3GPPAudioSampleEntryBox *) gf_isom_box_new(cfg->type);
		if (!entry) return GF_OUT_OF_MEM;
		entry->info = (GF_3GPPConfigBox *) gf_isom_box_new(cfg_type);
		if (!entry->info) {
			gf_isom_box_del((GF_Box *) entry);
			return GF_OUT_OF_MEM;
		}
		memcpy(&entry->info->cfg, cfg, sizeof(GF_3GPConfig));
		entry->samplerate_hi = trak->Media->mediaHeader->timeScale;
		entry->dataReferenceIndex = dataRefIndex;
		e = gf_list_add(trak->Media->information->sampleTable->SampleDescription->boxList, entry);
		*outDescriptionIndex = gf_list_count(trak->Media->information->sampleTable->SampleDescription->boxList);
	}
		break;
	case GF_ISOM_SUBTYPE_3GP_H263:
	{
		GF_3GPPVisualSampleEntryBox *entry = (GF_3GPPVisualSampleEntryBox *) gf_isom_box_new(cfg->type);
		if (!entry) return GF_OUT_OF_MEM;
		entry->info = (GF_3GPPConfigBox *) gf_isom_box_new(cfg_type);
		if (!entry->info) {
			gf_isom_box_del((GF_Box *) entry);
			return GF_OUT_OF_MEM;
		}
		memcpy(&entry->info->cfg, cfg, sizeof(GF_3GPConfig));
		entry->dataReferenceIndex = dataRefIndex;
		e = gf_list_add(trak->Media->information->sampleTable->SampleDescription->boxList, entry);
		*outDescriptionIndex = gf_list_count(trak->Media->information->sampleTable->SampleDescription->boxList);
	}
		break;
	}
	return e;
}

GF_Err gf_isom_3gp_config_update(GF_ISOFile *the_file, u32 trackNumber, GF_3GPConfig *param, u32 DescriptionIndex)
{
	GF_TrackBox *trak;
	GF_Err e;
	GF_3GPConfig *cfg;
	GF_3GPPAudioSampleEntryBox *entry;

	e = CanAccessMovie(the_file, GF_ISOM_OPEN_WRITE);
	if (e) return e;
	
	trak = gf_isom_get_track_from_file(the_file, trackNumber);
	if (!trak || !trak->Media || !param || !DescriptionIndex) return GF_BAD_PARAM;

	cfg = NULL;
	entry = (GF_3GPPAudioSampleEntryBox *)gf_list_get(trak->Media->information->sampleTable->SampleDescription->boxList, DescriptionIndex-1);
	if (!entry) return GF_BAD_PARAM;
	switch (entry->type) {
	case GF_ISOM_SUBTYPE_3GP_AMR:
	case GF_ISOM_SUBTYPE_3GP_AMR_WB:
	case GF_ISOM_SUBTYPE_3GP_EVRC:
	case GF_ISOM_SUBTYPE_3GP_QCELP:
	case GF_ISOM_SUBTYPE_3GP_SMV:
		cfg = &entry->info->cfg;
		break;
	case GF_ISOM_SUBTYPE_3GP_H263:
		cfg = & ((GF_3GPPVisualSampleEntryBox *)entry)->info->cfg;
		break;
	default:
		break;
	}
	if (!cfg || (cfg->type != param->type)) return GF_BAD_PARAM;
	memcpy(cfg, param, sizeof(GF_3GPConfig));
	return GF_OK;
}


#endif	//GPAC_READ_ONLY

