/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * himpp-video-viu.cpp
 * Copyright (C) 2015 Watson Xu <xuhuashan@gmail.com>
 *
 * live-streamer is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * live-streamer is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <himpp-common.h>
#include "himpp-video-viu.h"


HimppViDev::HimppViDev(HimppObject *source, HimppVideoSensor *sensor, VI_DEV vi_dev)
    : HimppVideoObject(source), _video_sensor(sensor)
      , _video_isp(sensor), _devid(vi_dev)
{
    _brightness = 50;
    _contrast = 50;
    _chroma = 50;
    _saturation = 50;
}

HimppViDev::~HimppViDev()
{
}

uint32_t HimppViDev::getBrightness()
{
    return _brightness;
}

void HimppViDev::setBrightness(uint32_t val)
{
    if (isEnabled()) {
        VI_CSC_ATTR_S csc_attr;
        if (HI_MPI_VI_GetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("get CSC attr failed");

        csc_attr.u32LumaVal = val;
        if (HI_MPI_VI_SetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("set CSC attr failed");
    }

    _brightness = val;
}

uint32_t HimppViDev::getContrast()
{
    return _contrast;
}

void HimppViDev::setContrast(uint32_t val)
{
    if (isEnabled()) {
        VI_CSC_ATTR_S csc_attr;
        if (HI_MPI_VI_GetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("get CSC attr failed");

        csc_attr.u32ContrVal = val;
        if (HI_MPI_VI_SetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("set CSC attr failed");
    }

    _contrast = val;
}

uint32_t HimppViDev::getChroma()
{
    return _chroma;
}

void HimppViDev::setChroma(uint32_t val)
{
    if (isEnabled()) {
        VI_CSC_ATTR_S csc_attr;
        if (HI_MPI_VI_GetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("get CSC attr failed");

        csc_attr.u32HueVal = val;
        if (HI_MPI_VI_SetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("set CSC attr failed");
    }

    _chroma = val;
}

uint32_t HimppViDev::getSaturation()
{
    return _saturation;
}

void HimppViDev::setSaturation(uint32_t val)
{
    if (isEnabled()) {
        VI_CSC_ATTR_S csc_attr;
        if (HI_MPI_VI_GetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("get CSC attr failed");

        csc_attr.u32SatuVal = val;
        if (HI_MPI_VI_SetCSCAttr(_devid, &csc_attr) != HI_SUCCESS)
            throw IpcamError("set CSC attr failed");
    }

    _saturation = val;
}

bool HimppViDev::enableObject()
{
    HI_S32 s32Ret;
    VI_DEV_ATTR_S *dev_attr;

    if (!_video_isp.enable())
        goto err_free_isp;

    dev_attr = *_video_sensor;
    if ((s32Ret = HI_MPI_VI_SetDevAttr(_devid, dev_attr)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_SetDevAttr %d failed [%#x]\n", _devid, s32Ret);
        goto err_disable_isp;
    }

    if ((s32Ret = HI_MPI_VI_EnableDev(_devid)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_EnableDev %d failed [%#x]\n", _devid, s32Ret);
        goto err_disable_isp;
    }

    VI_CSC_ATTR_S csc_attr;
    csc_attr.enViCscType = VI_CSC_TYPE_709;
    csc_attr.u32LumaVal = _brightness;
    csc_attr.u32ContrVal = _contrast;
    csc_attr.u32HueVal = _chroma;
    csc_attr.u32SatuVal = _saturation;
    if ((s32Ret = HI_MPI_VI_SetCSCAttr(_devid, &csc_attr)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_SetCSCAttr %d failed [%#x]\n", _devid, s32Ret);
    }

    return true;

err_disable_isp:
    _video_isp.disable();

err_free_isp:
    return false;
}

bool HimppViDev::disableObject()
{
    HI_MPI_VI_DisableDev(_devid);
    _video_isp.disable();

    return true;
}

bool HimppViDev::setResolution(ImageResolution &res)
{
    return false;
}

ImageResolution HimppViDev::getResolution()
{
    ISP_IMAGE_ATTR_S *attr = *_video_sensor;
    return ImageResolution(attr->u16Width, attr->u16Height);
}

bool HimppViDev::setFramerate(uint32_t fps)
{
    return false;
}

uint32_t HimppViDev::getFramerate()
{
    ISP_IMAGE_ATTR_S *attr = *_video_sensor;
    return attr->u16FrameRate;
}

HimppViDev::operator MPP_CHN_S* ()
{
    return NULL;
}

HimppViDev::operator HimppVideoISP* ()
{
    return &_video_isp;
}

//////////////////////////////////////////////////////////////////////////////

HimppViChan::HimppViChan(HimppViDev *vi_dev, VI_CHN vi_chn)
    : HimppVideoObject(vi_dev),
      _vi_dev(vi_dev), _chnid(vi_chn), _rotate(ROTATE_NONE)
{
    _mpp_chn.enModId = HI_ID_VIU;
    _mpp_chn.s32DevId = vi_dev->getDeviceId();
    _mpp_chn.s32ChnId = _chnid;

    _ldc_attr.bEnable = HI_FALSE;
    _ldc_attr.stAttr.enViewType = LDC_VIEW_TYPE_ALL;
    _ldc_attr.stAttr.s32CenterXOffset = 0;
    _ldc_attr.stAttr.s32CenterYOffset = 0;
    _ldc_attr.stAttr.s32Ratio = 255;

    _flip = false;
    _mirror = false;
}

HimppViChan::~HimppViChan()
{
}

bool HimppViChan::enableObject()
{
    HI_U32 s32Ret;
    VI_CHN_ATTR_S   attr;
    ImageResolution in_res = _vi_dev->getResolution();

    attr.stCapRect.s32X = 0;
    attr.stCapRect.s32Y = 0;
    attr.stCapRect.u32Width = in_res.Width;
    attr.stCapRect.u32Height = in_res.Height;
    attr.stDestSize.u32Width = in_res.Width;
    attr.stDestSize.u32Height = in_res.Height;
    attr.enCapSel = VI_CAPSEL_BOTH;
    attr.enPixFormat = HIMPP_PIXEL_FORMAT;
    attr.bMirror = (HI_BOOL)_mirror;
    attr.bFlip = (HI_BOOL)_flip;
    attr.bChromaResample = HI_FALSE;
    attr.s32SrcFrameRate = -1;
    attr.s32FrameRate = -1;

    if ((s32Ret = HI_MPI_VI_SetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_SetChnAttr %d failed [%#x]\n",
                    _chnid, s32Ret);
        return false;
    }

    if ((s32Ret = HI_MPI_VI_SetRotate(_chnid, _rotate)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_SetRotate %d failed [%#x]\n",
                    _chnid, s32Ret);
        return false;
    }

    if ((s32Ret = HI_MPI_VI_EnableChn(_chnid)) != HI_SUCCESS) {
        HIMPP_PRINT("HI_MPI_VI_EnableChn %d failed [%#x]\n",
                    _chnid, s32Ret);
        return false;
    }

    if (_chnid < VIU_MAX_PHYCHN_NUM) {
        if ((s32Ret = HI_MPI_VI_SetLDCAttr(_chnid, &_ldc_attr)) != HI_SUCCESS) {
            HIMPP_PRINT("HI_MPI_VI_SetLDCAttr %d failed [%#x]\n",
                        _chnid, s32Ret);
        }
    }

    return true;
}

bool HimppViChan::disableObject()
{
    return (HI_MPI_VI_DisableChn(_chnid) == HI_SUCCESS);
}

bool HimppViChan::setResolution(ImageResolution &res)
{
    return false;
}

ImageResolution HimppViChan::getResolution()
{
    return _vi_dev->getResolution();
}

bool HimppViChan::setFramerate(uint32_t fps)
{
    return false;
}

uint32_t HimppViChan::getFramerate()
{
    return _vi_dev->getFramerate();
}

HimppViChan::operator MPP_CHN_S* ()
{
    return &_mpp_chn;
}

ROTATE_E HimppViChan::getRotate()
{
    return _rotate;
}

void HimppViChan::setRotate(ROTATE_E value)
{
    if (value < 0 || value >= ROTATE_BUTT)
        throw IpcamError("Invalid Value");

    if (isEnabled()) {
        ROTATE_E old_val = _rotate;
        _rotate = value;
        if (!disableObject() || !enableObject()) {
            _rotate = old_val;
            disableObject();
            enableObject();
            throw IpcamError("Failed to set property");
        }
    }

    _rotate = value;
}

bool HimppViChan::getMirror()
{
    return _mirror;
}

void HimppViChan::setMirror(bool value)
{
    HI_U32 s32Ret;
    VI_CHN_ATTR_S   attr;

    if (isEnabled()) {
        if ((s32Ret = HI_MPI_VI_GetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
            throw IpcamError("Failed to get property");
        }
        attr.bMirror = (HI_BOOL)value;
        if ((s32Ret = HI_MPI_VI_SetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
            throw IpcamError("Failed to set property");
        }
    }

    _mirror = value;
}

bool HimppViChan::getFlip()
{
    return _flip;
}

void HimppViChan::setFlip(bool value)
{
    HI_U32 s32Ret;
    VI_CHN_ATTR_S   attr;

    if (isEnabled()) {
        if ((s32Ret = HI_MPI_VI_GetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
            throw IpcamError("Failed to get property");
        }
        attr.bFlip = (HI_BOOL)value;
        if ((s32Ret = HI_MPI_VI_SetChnAttr(_chnid, &attr)) != HI_SUCCESS) {
            throw IpcamError("Failed to set property");
        }
    }

    _flip = value;
}
