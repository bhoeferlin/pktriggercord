#ifndef __PENTAXTETHERLIB_H__
#define __PENTAXTETHERLIB_H__

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>


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
		EXPOSURE_MODE_P = 1,
		EXPOSURE_MODE_SV,
		EXPOSURE_MODE_TV,
		EXPOSURE_MODE_AV,
		EXPOSURE_MODE_TAV,
		EXPOSURE_MODE_M,
		EXPOSURE_MODE_B,
		EXPOSURE_MODE_X
	};

    enum AutoExposureMode
    {
        AUTOEXPOSURE_METERING_INVALID = -1,
        AUTOEXPOSURE_METERING_MULTI = 0,
        AUTOEXPOSURE_METERING_CENTER,
        AUTOEXPOSURE_METERING_SPOT
    };


	enum AutoFocusMode
	{
		AF_MODE_INVALID = -1,
		AF_MODE_MF = 0,
		AF_MODE_AF_S,
		AF_MODE_AF_C,
		AF_MODE_AF_A
	};

	enum AutoFocusPointSelectionMode
	{
		AF_POINT_SELECTION_INVALID = -1,
		AF_POINT_SELECTION_SPOT = 0,
		AF_POINT_SELECTION_SELECT_1 = 1,
		AF_POINT_SELECTION_SELECT_9 = 2,
		AF_POINT_SELECTION_SELECT_25 = 3,
		AF_POINT_SELECTION_SELECT_27 = 4,
		AF_POINT_SELECTION_AUTO_9 = 5, 
		AF_POINT_SELECTION_AUTO_27 = 6,  
		AF_POINT_SELECTION_AUTO_5 = 7,
		AF_POINT_SELECTION_AUTO_11 = 8
	};


    enum ColorDynamicsMode 
    {
        COLOR_DYNAMICS_INVALID = -1,
        COLOR_DYNAMICS_NATURAL,
        COLOR_DYNAMICS_BRIGHT,
        COLOR_DYNAMICS_PORTRAIT,
        COLOR_DYNAMICS_LANDSCAPE,
        COLOR_DYNAMICS_VIBRANT,
        COLOR_DYNAMICS_MONOCHROME,
        COLOR_DYNAMICS_MUTED,
        COLOR_DYNAMICS_REVERSAL_FILM,
        COLOR_DYNAMICS_BLEACH_BYPASS,
        COLOR_DYNAMICS_RADIANT,
        COLOR_DYNAMICS_CROSS_PROCESSING,
        COLOR_DYNAMICS_FLAT,
        COLOR_DYNAMICS_AUTO
    };

    enum WhiteBalanceMode
    {
        WHITE_BALANCE_MODE_INVALID = -1,
        WHITE_BALANCE_MODE_AUTO = 0,
        WHITE_BALANCE_MODE_DAYLIGHT,
        WHITE_BALANCE_MODE_SHADE,
        WHITE_BALANCE_MODE_CLOUDY,
        WHITE_BALANCE_MODE_FLUORESCENT_DAYLIGHT_COLOR,
        WHITE_BALANCE_MODE_FLUORESCENT_DAYLIGHT_WHITE,
        WHITE_BALANCE_MODE_FLUORESCENT_COOL_WHITE,
        WHITE_BALANCE_MODE_TUNGSTEN,
        WHITE_BALANCE_MODE_FLASH,
        WHITE_BALANCE_MODE_MANUAL,
        WHITE_BALANCE_MODE_MANUAL_2,
        WHITE_BALANCE_MODE_MANUAL_3,
        WHITE_BALANCE_MODE_KELVIN_1,
        WHITE_BALANCE_MODE_KELVIN_2,
        WHITE_BALANCE_MODE_KELVIN_3,
        WHITE_BALANCE_MODE_FLUORESCENT_WARM_WHITE,
        WHITE_BALANCE_MODE_CTE,
        WHITE_BALANCE_MODE_MULTI_AUTO
    };

    enum FlashMode
    {
        FLASH_MODE_INVALID = -1,
        FLASH_MODE_ON = 0,
        FLASH_MODE_ON_REDEYE = 1,
        FLASH_MODE_SLOW = 2,
        FLASH_MODE_SLOW_REDEYE = 3,
        FLASH_MODE_FIRST_TRAILING_CURTAIN = 4,
        FLASH_MODE_AUTO = 5,
        FLASH_MODE_AUTO_REDEYE = 6,
        FLASH_MODE_SECOND_TRAILING_CURTAIN = 7,
        FLASH_MODE_WIRELESS_MASTER = 8,
        FLASH_MODE_WIRELESS_CONTROL = 9,
        FLASH_MODE_MANUAL = 10
    };

    enum ReleaseMode 
    {
        RELEASE_MODE_INVALID = -1,
        RELEASE_MODE_SINGLE = 0,
        RELEASE_DRIVE_MODE_CONTINUOUS_HI = 1,
        RELEASE_DRIVE_MODE_SELF_TIMER_12 = 2,
        RELEASE_DRIVE_MODE_SELF_TIMER_2 = 3,
        RELEASE_DRIVE_MODE_REMOTE = 4,
        RELEASE_DRIVE_MODE_REMOTE_3 = 5,
        RELEASE_DRIVE_MODE_CONTINUOUS_LO = 6,
        RELEASE_DRIVE_MODE_REMOTE_CONTINUOUS = 7,
        RELEASE_DRIVE_MODE_CONTINUOUS_MED = 8
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

    /**
     * @param cancelationFlag - if not null, this atomic bool will be stored as member (attention on scope!) and used
     *                          to test if connect and reconnects should be cancelled.
     */
	bool connect(unsigned int timeout_ms, std::atomic<bool>* cancelationFlag = nullptr);
	void disconnect();
	bool isConnected() const;
	uint32_t registerConnectionChangedCallback(const std::function<void(bool)>& callback);


	std::string getCameraName();
	std::string getFirmware();
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

	PentaxTetherLib::Rational<uint32_t> getFocalLength(bool forceStatusUpdate = false);
	uint32_t registerFocalLengthChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	double getExposureValue(bool forceStatusUpdate = false);
	uint32_t registerExposureValueChangedCallback(const std::function<void(double)>& callback);

	PentaxTetherLib::AutoFocusMode getAutoFocusMode(bool forceStatusUpdate = false);
	bool setAutoFocusMode(const PentaxTetherLib::AutoFocusMode& af_mode);
	uint32_t registerAutoFocusModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusMode&)>& callback);

	uint32_t getNumberOfAutoFocusPoints();
	PentaxTetherLib::AutoFocusPointSelectionMode getAutoFocusPointSelectionMode(bool forceStatusUpdate = false);
	bool setAutoFocusPointSelectionMode(const PentaxTetherLib::AutoFocusPointSelectionMode& af_mode);
	uint32_t registerAutoFocusPointSelectionModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusPointSelectionMode&)>& callback);

	std::vector<uint32_t> getSelectedAutoFocusPointIndex(bool forceStatusUpdate = false);
	bool setSelectedAutoFocusPointIndex(const std::vector<uint32_t>& af_point_idx);
	uint32_t registerSelectedAutoFocusPointChangedCallback(const std::function<void(const std::vector<uint32_t>&)>& callback);


    PentaxTetherLib::ColorDynamicsMode getColorDynamicsMode(bool forceStatusUpdate = false);
    bool setColorDynamicsMode(const PentaxTetherLib::ColorDynamicsMode& ColorDynamicsMode);
    uint32_t registerColorDynamicsModeChangedCallback(const std::function<void(const PentaxTetherLib::ColorDynamicsMode&)>& callback);

    int32_t getToneSaturation(bool forceStatusUpdate = false);
    bool setToneSaturation(const int32_t& saturation);
    std::pair<int32_t, int32_t> getToneSaturationLimits();
    uint32_t registerToneSaturationChangedCallback(const std::function<void(const int32_t&)>& callback);

    int32_t getToneHue(bool forceStatusUpdate = false);
    bool setToneHue(const int32_t& hue);
    std::pair<int32_t, int32_t> getToneHueLimits();
    uint32_t registerToneHueChangedCallback(const std::function<void(const int32_t&)>& callback);

    int32_t getToneContrast(bool forceStatusUpdate = false);
    bool setToneContrast(const int32_t& contrast);
    std::pair<int32_t, int32_t> getToneContrastLimits();
    uint32_t registerToneContrastChangedCallback(const std::function<void(const int32_t&)>& callback);

    int32_t getToneSharpness(bool forceStatusUpdate = false);
    bool setToneSharpness(const int32_t& sharpness);
    std::pair<int32_t, int32_t> getToneSharpnessLimits();
    uint32_t registerToneSharpnessChangedCallback(const std::function<void(const int32_t&)>& callback);

    PentaxTetherLib::AutoExposureMode getAutoExposureMeteringMode(bool forceStatusUpdate = false);
    bool setAutoExposureMeteringMode(const PentaxTetherLib::AutoExposureMode& ae_mode);
    uint32_t registerAutoExposureMeteringModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoExposureMode&)>& callback);

    PentaxTetherLib::WhiteBalanceMode getWhiteBalanceMode(bool forceStatusUpdate = false);
    bool setWhiteBalanceMode(const PentaxTetherLib::WhiteBalanceMode& wb_mode);
    uint32_t registerWhiteBalanceModeChangedCallback(const std::function<void(const PentaxTetherLib::WhiteBalanceMode&)>& callback);

    /**
     * Returns pair of adjustment values [-getWhiteBalanceAdjustmentSteps, +getWhiteBalanceAdjustmentSteps].
     * The first part of the pair contains the magenta/green axis, the second the blue/amber axis.
     * Negative values, thus indicate magenta or blue deviation, positive values green or amber deviation.
     */
    std::pair<int32_t, int32_t> getWhiteBalanceAdjustment(bool forceStatusUpdate = false);
    std::pair<int32_t, int32_t> getWhiteBalanceAdjustmentRange();
    bool setWhiteBalanceAdjustment(const int32_t& magenta_green, const int32_t& blue_amber);
    uint32_t registerWhiteBalanceAdjustmentChangedCallback(const std::function<void(const int32_t&, const int32_t&)>& callback);

    PentaxTetherLib::FlashMode getFlashMode(bool forceStatusUpdate = false);
    bool setFlashMode(const PentaxTetherLib::FlashMode& flash_mode);
    uint32_t registerFlashModeChangedCallback(const std::function<void(const PentaxTetherLib::FlashMode&)>& callback);

    PentaxTetherLib::Rational<int32_t> getFlashExposureCompensation(bool forceStatusUpdate = false);
    bool setFlashExposureCompensation(const PentaxTetherLib::Rational<int32_t>& shutterTime);
    std::vector<PentaxTetherLib::Rational<int32_t>> getFlashExposureCompensationSteps();
    uint32_t registerFlashExposureCompensationChangedCallback(const std::function<void(const PentaxTetherLib::Rational<int32_t>&)>& callback);
    
    bool getShakeReduction(bool forceStatusUpdate = false);
    uint32_t registerShakeReductionChangedCallback(const std::function<void(bool)>& callback);

    PentaxTetherLib::ReleaseMode getReleaseMode(bool forceStatusUpdate = false);
    bool setReleaseMode(const PentaxTetherLib::ReleaseMode& release_mode);
    uint32_t registerReleaseModeChangedCallback(const std::function<void(const PentaxTetherLib::ReleaseMode&)>& callback);




	//! Actions

	/**
	* Runs the auto focus.
	* @return - vector of (zero-based) index auto focus points that were used to focus
	*/
	std::vector<uint32_t> executeFocus();

	/**
	 * Triggers the camera shutter.
	 * @return - the index of the camera buffer to which the image was stored
	 */
	uint32_t executeShutter();

    /**
     * Executes the dust removal program
     */
	bool executeDustRemoval();



	std::vector<uint8_t> getImage( int bufferIndex, ImageFormat format = IF_CURRENT_CAM_SETTING, JpgQuality jpgQuality = JPEG_CURRENT_CAM_SETTING, ImageResolution resolution = RES_CURRENT_CAM_SETTING, std::function<void(float)> progressCallback = nullptr );

	std::vector<uint8_t> getPreviewImage(int bufferIndex);

	void unregisterCallback(const uint32_t& callbackIdentifier);



private:

	class Impl;                     
	std::unique_ptr<Impl> impl_;


};

#endif

