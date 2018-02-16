#ifndef __PENTAXTETHERLIB_H__
#define __PENTAXTETHERLIB_H__

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>


#if defined(PENTAX_TETHER_LIB_EXPORT) 
#   define PENTAX_TETHER_API   __declspec(dllexport)
#else 
#   define PENTAX_TETHER_API   __declspec(dllimport)
#endif  


class PENTAX_TETHER_API PentaxTetherLib
{
public:

	enum ImageFormat
	{
		IF_CURRENT_CAM_SETTING = -1,
		IF_PEF = 0, // Native Pentax RAW
		IF_DNG = 1, // Open but commercial Adobe Format for unification
		IF_JPG = 2
	};

	enum JpgQuality
	{
		JPEG_CURRENT_CAM_SETTING = -1,
		JPEG_LOW = 1,
		JPEG_MEDIUM = 2,
		JPEG_HIGH = 3,
		JPEG_BEST = 4
	};

	enum ImageResolution
	{
		RES_CURRENT_CAM_SETTING = -1,
		RES_L = 0,
		RES_M = 1,
		RES_S = 2,
		RES_XS = 3
	};

	struct Options
	{
		// options - to be made configurable
		bool reconnect_{ true };
		unsigned int reconnectionTimeout_{ 60 };
		double statusMaxAge_sec{ 0.5 };
	};



	static const uint32_t InvalidBufferIndex{ 8 * sizeof(uint16_t) };


	PentaxTetherLib::PentaxTetherLib(const PentaxTetherLib::Options& options);
	PentaxTetherLib::~PentaxTetherLib();
	PentaxTetherLib::PentaxTetherLib(PentaxTetherLib &&) noexcept = delete;
	PentaxTetherLib& PentaxTetherLib::operator=(PentaxTetherLib &&) noexcept = delete;

	bool connect(unsigned int timeout_ms);
	void disconnect();
	bool isConnected() const;
	uint32_t registerConnectionChangedCallback(const std::function<void(bool)>& callback);


	std::string getCameraName();
	
	// properties
	uint32_t getISO( bool forceStatusUpdate = false );
	bool setISO(uint32_t isoValue);
	std::vector<uint32_t> getISOSteps();
	uint32_t registerISOChangedCallback( const std::function<void(uint32_t)>& callback );

	// actions
	bool executeFocus();
	uint32_t executeShutter();

	std::vector<uint8_t> getImage( int bufferIndex, ImageFormat format = IF_CURRENT_CAM_SETTING, JpgQuality jpgQuality = JPEG_CURRENT_CAM_SETTING, ImageResolution resolution = RES_CURRENT_CAM_SETTING, std::function<void(float)> progressCallback = nullptr );


	void unregisterCallback(const uint32_t& callbackIdentifier);



private:

	class Impl;                     
	std::unique_ptr<Impl> impl_;    




};

#endif

