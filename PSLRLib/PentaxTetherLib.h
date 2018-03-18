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

	enum ExposureMode 
	{
		EXPOSURE_MODE_INVALID = -2,
		EXPOSURE_MODE_GREEN = 0,
		EXPOSURE_MODE_P,
		EXPOSURE_MODE_SV,
		EXPOSURE_MODE_TV,
		EXPOSURE_MODE_AV,
		EXPOSURE_MODE_TAV,
		EXPOSURE_MODE_M,
		EXPOSURE_MODE_B,
		EXPOSURE_MODE_X
	};

	struct Options
	{
		// options - to be made configurable
		bool reconnect{ true };
		unsigned int reconnectionTimeout{ 60 };
		double statusMaxAge_sec{ 0.5 };
	};

	struct ISOSettings
	{
		uint32_t fixedISOValue{ 0 };
		uint32_t autoMinimumISOValue{ 0 };
		uint32_t autoMaximumISOValue{ 0 };
		bool isoFixed() { return fixedISOValue > 0; };
		bool isValid() { return (fixedISOValue > 0) ^ (autoMinimumISOValue > 0 && autoMaximumISOValue > 0 && autoMinimumISOValue <= autoMinimumISOValue); };

	};

	template<typename T>
	struct Rational
	{
		static_assert(std::is_integral<T>::value, "Integer type expected!");

		Rational() : nominator(0), denominator(0) {};
		Rational(T nom, T den) : nominator(nom), denominator(den) {};

		T nominator;
		T denominator;

		double toDouble() const
		{
			if (denominator != 0.0)
			{
				return static_cast<double>(nominator) / static_cast<double>(denominator);
			}
			else
			{
				return std::numeric_limits<double>::quiet_NaN();
			}
		}


		bool isInvalid() const
		{
			return nominator == 0 && denominator == 0;
		}

		bool operator== (const PentaxTetherLib::Rational<T>& a) const
		{
			if (a.isInvalid() && this->isInvalid())
			{
				// Test for invalid passes - all other rationals with denminator zero will fail this equality test
				return true;
			}
			else
			{
				return a.toDouble() == this->toDouble();
			}
		}

		bool operator!= (const PentaxTetherLib::Rational<T>& a) const
		{
			return !(*this == a);
		}
	};

	static constexpr int32_t InvalidBufferIndex{ -1 };


	PentaxTetherLib::PentaxTetherLib(const PentaxTetherLib::Options& options);
	PentaxTetherLib::~PentaxTetherLib();
	PentaxTetherLib::PentaxTetherLib(PentaxTetherLib &&) noexcept = delete;
	PentaxTetherLib& PentaxTetherLib::operator=(PentaxTetherLib &&) noexcept = delete;

	bool connect(unsigned int timeout_ms);
	void disconnect();
	bool isConnected() const;
	uint32_t registerConnectionChangedCallback(const std::function<void(bool)>& callback);


	std::string getCameraName();
	std::string getLensType();
	
	// properties
	uint32_t getISO( bool forceStatusUpdate = false );
	bool setFixedISO(uint32_t isoValue);
	bool setAutoISORange(uint32_t minISOValue, uint32_t maxISOValue);
	ISOSettings getISOSettings();
	std::vector<uint32_t> getISOSteps();
	uint32_t registerISOChangedCallback( const std::function<void(uint32_t)>& callback );

	PentaxTetherLib::Rational<uint32_t> getAperture(bool forceStatusUpdate = false);
	bool setAperture(const PentaxTetherLib::Rational<uint32_t>& apertureValue);
	std::vector<PentaxTetherLib::Rational<uint32_t>> getApertureSteps();
	uint32_t registerApertureChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	PentaxTetherLib::Rational<uint32_t> getShutterTime(bool forceStatusUpdate = false);
	bool setShutterTime(const PentaxTetherLib::Rational<uint32_t>& shutterTime);
	std::vector<PentaxTetherLib::Rational<uint32_t>> getShutterTimeSteps();
	uint32_t registerShutterTimeChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	PentaxTetherLib::Rational<int32_t> getExposureCompensation(bool forceStatusUpdate = false);
	bool setExposureCompensation(const PentaxTetherLib::Rational<int32_t>& shutterTime);
	std::vector<PentaxTetherLib::Rational<int32_t>> getExposureCompensationSteps();
	uint32_t registerExposureCompensationChangedCallback(const std::function<void(const PentaxTetherLib::Rational<int32_t>&)>& callback);

	PentaxTetherLib::ExposureMode getExposureMode();
	uint32_t registerExposureModeChangedCallback(const std::function<void(const PentaxTetherLib::ExposureMode&)>& callback);

	std::vector<float> getBatteryVoltage(bool forceStatusUpdate = false);
	uint32_t registerBatteryVoltageChangedCallback(const std::function<void(const std::vector<float>&)>& callback);

	// actions
	bool executeFocus();
	uint32_t executeShutter();

	std::vector<uint8_t> getImage( int bufferIndex, ImageFormat format = IF_CURRENT_CAM_SETTING, JpgQuality jpgQuality = JPEG_CURRENT_CAM_SETTING, ImageResolution resolution = RES_CURRENT_CAM_SETTING, std::function<void(float)> progressCallback = nullptr );

	std::vector<uint8_t> getPreviewImage(int bufferIndex);

	void unregisterCallback(const uint32_t& callbackIdentifier);



private:

	class Impl;                     
	std::unique_ptr<Impl> impl_;    




};

#endif

