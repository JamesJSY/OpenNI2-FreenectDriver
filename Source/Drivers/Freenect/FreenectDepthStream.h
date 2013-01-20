#ifndef _FREENECT_DEPTH_STREAM_H_
#define _FREENECT_DEPTH_STREAM_H_

#include "FreenectVideoStream.h"
#include "Driver/OniDriverAPI.h"
#include "libfreenect.hpp"


class FreenectDepthStream : public FreenectVideoStream
{
protected:
	typedef std::map< OniVideoMode, std::pair<freenect_depth_format, freenect_resolution> > FreenectDepthModeMap;
	OniImageRegistrationMode image_registration_mode;
	
private:
	static const OniSensorType sensor_type = ONI_SENSOR_DEPTH;
	static FreenectDepthModeMap getSupportedVideoModes();
	virtual void populateFrame(void* data, OniDriverFrame* pFrame) const;
	OniStatus setVideoMode(OniVideoMode requested_mode)
	{
		FreenectDepthModeMap supported_video_modes = getSupportedVideoModes();
		FreenectDepthModeMap::const_iterator matched_mode_iter = supported_video_modes.find(requested_mode);
		if (matched_mode_iter == supported_video_modes.end())
			return ONI_STATUS_NOT_SUPPORTED;			
		
		freenect_depth_format format = matched_mode_iter->second.first;
		freenect_resolution resolution = matched_mode_iter->second.second;
		if (image_registration_mode == ONI_IMAGE_REGISTRATION_DEPTH_TO_COLOR) // force registration mode
			format = FREENECT_DEPTH_REGISTERED;
		
		try { device->setDepthFormat(format, resolution); }
		catch (std::runtime_error e)
		{
			printf("format-resolution combination not supported by libfreenect: %d-%d\n", format, resolution);
			return ONI_STATUS_NOT_SUPPORTED;
		}
		video_mode = requested_mode;
		return ONI_STATUS_OK;
	}

public:
	FreenectDepthStream(Freenect::FreenectDevice* pDevice);
	~FreenectDepthStream() { }
	
	static OniSensorInfo getSensorInfo()
	{
		FreenectDepthModeMap supported_modes = getSupportedVideoModes();
		OniVideoMode* modes = new OniVideoMode[supported_modes.size()];
		std::transform(supported_modes.begin(), supported_modes.end(), modes, RetrieveKey());
		return { sensor_type, SIZE(modes), modes }; // sensorType, numSupportedVideoModes, pSupportedVideoModes
	}
	virtual OniBool isImageRegistrationModeSupported(OniImageRegistrationMode mode);
	OniImageRegistrationMode getImageRegistrationMode() const { return image_registration_mode; }
	virtual OniStatus setImageRegistrationMode(OniImageRegistrationMode mode)
	{
		if (!isImageRegistrationModeSupported(mode))
			return ONI_STATUS_NOT_SUPPORTED;
		image_registration_mode = mode;
		return ONI_STATUS_OK;
	}
	
	// from StreamBase
	virtual OniStatus setProperty(int propertyId, const void* data, int dataSize);
};


#endif // _FREENECT_DEPTH_STREAM_H_
