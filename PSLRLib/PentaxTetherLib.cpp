#include "PentaxTetherLib.h"

extern "C"
{
	bool debug = false;
	bool warnings = false;

#include "../pslr.h"
#include "../pslr_lens.h"
}

#include <time.h>
#include <iostream>
#include <assert.h>
#include <map>
#include <thread>




int main()
{
	PentaxTetherLib::Options defaultOptions;
	PentaxTetherLib tetherLib( defaultOptions );
	if (tetherLib.connect(30))
	{
		std::cout << "Connected with: " << tetherLib.getCameraName() << std::endl;
		std::cout << "ISO: " << tetherLib.getISO() << std::endl;

		tetherLib.setFixedISO(100);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "ISO: " << tetherLib.getISO() << std::endl;

		tetherLib.setFixedISO(600);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "ISO: " << tetherLib.getISO() << std::endl;

		if (tetherLib.executeFocus().size() > 0)
		{
			std::cout << "Focus done!" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
			uint32_t bufferIndex = tetherLib.executeShutter();
			tetherLib.getPreviewImage(bufferIndex);

//			tetherLib.getImage(bufferIndex);
		}
		else
		{
			std::cout << "Focus NOT done!" << std::endl;
		}

	}
	else
	{
		std::cout << "Not connected!" << std::endl;
	}

	std::this_thread::sleep_for(std::chrono::seconds(10));

	return 0;
}






class PentaxTetherLib::Impl
{
public:
	Impl( const PentaxTetherLib::Options& options);
	~Impl();


	std::shared_ptr<pslr_status> pollStatus(bool forceStatusUpdate = false);
	bool testResult(const int& result);

	bool connect(unsigned int timeout_sec);
	void disconnect();
	bool isConnected() const;
	uint32_t registerConnectionChangedCallback(const std::function<void(bool)>& callback);

	std::string getCameraName();
	std::string getFirmware();
	std::string getLensType();


	std::vector<uint32_t> executeFocus();
	int32_t executeShutter();
	bool executeDustRemoval();


	std::vector<uint8_t> getImage(int bufferIndex, ImageFormat format, JpgQuality jpgQuality, ImageResolution resolution, std::function<void(float)> progressCallback);
	std::vector<uint8_t> getPreviewImage(int bufferIndex);

	PentaxTetherLib::ExposureMode getExposureMode(bool forceStatusUpdate = false);
	uint32_t registerExposureModeChangedCallback(const std::function<void(const PentaxTetherLib::ExposureMode&)>& callback);

	uint32_t getISO(bool forceStatusUpdate);
	bool setFixedISO(uint32_t isoValue);
	bool setAutoISORange(uint32_t minISOValue, uint32_t maxISOValue);
	PentaxTetherLib::ISOSettings getISOSettings(bool forceStatusUpdate);
	std::vector<uint32_t> getISOSteps(bool forceStatusUpdate = false);
	uint32_t registerISOChangedCallback(const std::function<void(uint32_t)>& callback);

	PentaxTetherLib::Rational<uint32_t> getAperture(bool forceStatusUpdate);
	bool setAperture(const PentaxTetherLib::Rational<uint32_t>& apertureValue);
	std::vector<PentaxTetherLib::Rational<uint32_t>> getApertureSteps(bool forceStatusUpdate = false);
	uint32_t registerApertureChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	PentaxTetherLib::Rational<uint32_t> getShutterTime(bool forceStatusUpdate);
	bool setShutterTime(const PentaxTetherLib::Rational<uint32_t>& shutterTime);
	std::vector<PentaxTetherLib::Rational<uint32_t>> getShutterTimeSteps(bool forceStatusUpdate = false);
	uint32_t registerShutterTimeChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	PentaxTetherLib::Rational<int32_t> getExposureCompensation(bool forceStatusUpdate);
	bool setExposureCompensation(const PentaxTetherLib::Rational<int32_t>& shutterTime);
	std::vector<PentaxTetherLib::Rational<int32_t>> getExposureCompensationSteps(bool forceStatusUpdate = false);
	uint32_t registerExposureCompensationChangedCallback(const std::function<void(const PentaxTetherLib::Rational<int32_t>&)>& callback);

	std::vector<float> getBatteryVoltage(bool forceStatusUpdate);
	uint32_t registerBatteryVoltageChangedCallback(const std::function<void(const std::vector<float>&)>& callback);

	PentaxTetherLib::Rational<uint32_t> getFocalLength(bool forceStatusUpdate);
	uint32_t registerFocalLengthChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback);

	double getExposureValue(bool forceStatusUpdate);
	uint32_t registerExposureValueChangedCallback(const std::function<void(double)>& callback);

	PentaxTetherLib::AutoFocusMode getAutoFocusMode(bool forceStatusUpdate);
	bool setAutoFocusMode(const PentaxTetherLib::AutoFocusMode& af_mode);
	uint32_t registerAutoFocusModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusMode&)>& callback);

	uint32_t getNumberOfAutoFocusPoints();
	PentaxTetherLib::AutoFocusPointSelectionMode getAutoFocusPointSelectionMode(bool forceStatusUpdate);
	bool setAutoFocusPointSelectionMode(const PentaxTetherLib::AutoFocusPointSelectionMode& af_mode);
	uint32_t registerAutoFocusPointSelectionModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusPointSelectionMode&)>& callback);

	std::vector<uint32_t> getSelectedAutoFocusPointIndex(bool forceStatusUpdate );
	bool setSelectedAutoFocusPointIndex(const std::vector<uint32_t>& af_point_idx);
	uint32_t registerSelectedAutoFocusPointChangedCallback(const std::function<void(const std::vector<uint32_t>&)>& callback);

	void unregisterCallback(const uint32_t& callbackIdentifier);

private:

	void processStatusCallbacks();

	// conversions
	template< typename T>
	static PentaxTetherLib::Rational<T> fromPSLR(const pslr_rational_t& r);
	template< typename T>
	static pslr_rational_t toPSLR(const PentaxTetherLib::Rational<T>& r);
	static PentaxTetherLib::ExposureMode fromPSLR(const pslr_gui_exposure_mode_t& e);
	static pslr_gui_exposure_mode_t toPSLR(const PentaxTetherLib::ExposureMode& e);
	static PentaxTetherLib::AutoFocusMode fromPSLR(const pslr_af_mode_t& e);
	static pslr_af_mode_t toPSLR(const PentaxTetherLib::AutoFocusMode& e);
	static PentaxTetherLib::AutoFocusPointSelectionMode fromPSLR(const pslr_af_point_sel_t& e, const uint32_t& numberOfAFPoints);
	static pslr_af_point_sel_t toPSLR(const PentaxTetherLib::AutoFocusPointSelectionMode& e, const uint32_t& numberOfAFPoints);

	static std::vector<uint32_t> decodeAutoFocusPoints(const uint32_t& autoFocusFlagList, const uint32_t& numberOfAFPoints);
	static uint32_t encodeAutoFocusPoints(const std::vector<uint32_t>& af_point_indices, const uint32_t& numberOfAFPoints);

	static std::vector<float> batteryStateFromPSLR(const std::shared_ptr<pslr_status>& status);
	
	static double calculateExposureValue(const std::shared_ptr<pslr_status>& status);

	PentaxTetherLib::Options options_;

	void* camhandle_{ nullptr };
	std::mutex camCommunicationMutex_;


	std::recursive_mutex statusMutex_;
	std::shared_ptr<pslr_status> currentStatus_{ nullptr };
	std::shared_ptr<pslr_status> lastStatus_{ nullptr };
	time_t statusUpdateTime_{ 0 };

	std::thread polling_thread_;


	// callbacks
	std::recursive_mutex callbackMutex_;
	uint32_t nextCallbackIdentifier_{0};
	std::map< uint32_t, std::function<void(bool)> > connectionCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::ExposureMode&)> > exposureModeCallbacks_;
	std::map< uint32_t, std::function<void(uint32_t)> > isoCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::Rational<uint32_t>&)> > apertureCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::Rational<uint32_t>&)> > shutterTimeCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::Rational<int32_t>&)> > exposureCompensationCallbacks_;
	std::map< uint32_t, std::function<void(const std::vector<float>&)> > batteryVoltageCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::Rational<uint32_t>&)> > focalLengthCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::AutoFocusMode&)> > autoFocusModeCallbacks_;
	std::map< uint32_t, std::function<void(double)> > exposureValueCallbacks_;
	std::map< uint32_t, std::function<void(const PentaxTetherLib::AutoFocusPointSelectionMode&)> > autoFocusPointSelectionModeCallbacks_;
	std::map< uint32_t, std::function<void(const std::vector<uint32_t>&)> > selectedAutoFocusPointIndexCallbacks_;

};


PentaxTetherLib::PentaxTetherLib( const PentaxTetherLib::Options& options )
	: impl_(new Impl(options))
{
}



PentaxTetherLib::~PentaxTetherLib()
{
}



bool PentaxTetherLib::connect(unsigned int timeout_sec)
{
	return impl_->connect(timeout_sec);
}


void PentaxTetherLib::disconnect()
{
	impl_->disconnect();
}


bool PentaxTetherLib::isConnected() const
{
	return impl_->isConnected();
}


void PentaxTetherLib::unregisterCallback(const uint32_t& callbackIdentifier)
{
	impl_->unregisterCallback(callbackIdentifier);
}


uint32_t PentaxTetherLib::registerConnectionChangedCallback(const std::function<void(bool)>& callback)
{
	return impl_->registerConnectionChangedCallback(callback);
}


std::string PentaxTetherLib::getCameraName()
{
	return impl_->getCameraName();
}


std::string PentaxTetherLib::getFirmware()
{
	return impl_->getFirmware();
}


std::string PentaxTetherLib::getLensType()
{
	return impl_->getLensType();
}


std::vector<uint32_t> PentaxTetherLib::executeFocus()
{
	return impl_->executeFocus();
}


bool PentaxTetherLib::executeDustRemoval()
{
	return impl_->executeDustRemoval();
}


uint32_t PentaxTetherLib::executeShutter()
{
	return impl_->executeShutter();
}


std::vector<uint8_t> PentaxTetherLib::getImage
( 
	int bufferIndex, 
	ImageFormat format, 
	JpgQuality jpgQuality, 
	ImageResolution resolution, 
	std::function<void(float)> progressCallback
)
{
	return impl_->getImage(bufferIndex, format, jpgQuality, resolution, progressCallback);
}


std::vector<uint8_t> PentaxTetherLib::getPreviewImage(int bufferIndex)
{
	return impl_->getPreviewImage(bufferIndex);
}


uint32_t PentaxTetherLib::getISO(bool forceStatusUpdate)
{
	return impl_->getISO(forceStatusUpdate);
}


bool PentaxTetherLib::setFixedISO(uint32_t isoValue)
{
	return impl_->setFixedISO(isoValue);
}


bool PentaxTetherLib::setAutoISORange(uint32_t minISOValue, uint32_t maxISOValue)
{
	return impl_->setAutoISORange(minISOValue, maxISOValue);
}


PentaxTetherLib::ISOSettings PentaxTetherLib::getISOSettings()
{
	return impl_->getISOSettings(false);
}


std::vector<uint32_t> PentaxTetherLib::getISOSteps()
{
	return impl_->getISOSteps(false);
}


uint32_t PentaxTetherLib::registerISOChangedCallback(const std::function<void(uint32_t)>& callback)
{
	return impl_->registerISOChangedCallback(callback);
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::getAperture(bool forceStatusUpdate)
{
	return impl_->getAperture(forceStatusUpdate);
}


bool PentaxTetherLib::setAperture( const PentaxTetherLib::Rational<uint32_t>& apertureValue)
{
	return impl_->setAperture(apertureValue);
}


std::vector<PentaxTetherLib::Rational<uint32_t>> PentaxTetherLib::getApertureSteps()
{
	return impl_->getApertureSteps(false);
}


uint32_t PentaxTetherLib::registerApertureChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	return impl_->registerApertureChangedCallback(callback);
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::getShutterTime(bool forceStatusUpdate)
{
	return impl_->getShutterTime(forceStatusUpdate);
}


bool PentaxTetherLib::setShutterTime(const PentaxTetherLib::Rational<uint32_t>& shutterTime)
{
	return impl_->setShutterTime(shutterTime);
}


std::vector<PentaxTetherLib::Rational<uint32_t>> PentaxTetherLib::getShutterTimeSteps()
{
	return impl_->getShutterTimeSteps(false);
}


uint32_t PentaxTetherLib::registerShutterTimeChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	return impl_->registerShutterTimeChangedCallback(callback);
}


PentaxTetherLib::Rational<int32_t> PentaxTetherLib::getExposureCompensation(bool forceStatusUpdate)
{
	return impl_->getExposureCompensation(forceStatusUpdate);
}


bool PentaxTetherLib::setExposureCompensation(const PentaxTetherLib::Rational<int32_t>& shutterTime)
{
	return impl_->setExposureCompensation(shutterTime);
}


std::vector<PentaxTetherLib::Rational<int32_t>> PentaxTetherLib::getExposureCompensationSteps()
{
	return impl_->getExposureCompensationSteps(false);
}


uint32_t PentaxTetherLib::registerExposureCompensationChangedCallback(const std::function<void(const PentaxTetherLib::Rational<int32_t>&)>& callback)
{
	return impl_->registerExposureCompensationChangedCallback(callback);
}


PentaxTetherLib::ExposureMode PentaxTetherLib::getExposureMode()
{
	return impl_->getExposureMode(false);
}


uint32_t PentaxTetherLib::registerExposureModeChangedCallback(const std::function<void(const PentaxTetherLib::ExposureMode&)>& callback)
{
	return impl_->registerExposureModeChangedCallback(callback);
}


std::vector<float> PentaxTetherLib::getBatteryVoltage(bool forceStatusUpdate)
{
	return impl_->getBatteryVoltage(forceStatusUpdate);
}


uint32_t PentaxTetherLib::registerBatteryVoltageChangedCallback(const std::function<void(const std::vector<float>&)>& callback)
{
	return impl_->registerBatteryVoltageChangedCallback(callback);
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::getFocalLength(bool forceStatusUpdate)
{
	return impl_->getFocalLength(forceStatusUpdate);
}


uint32_t PentaxTetherLib::registerFocalLengthChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	return impl_->registerFocalLengthChangedCallback(callback);
}


double PentaxTetherLib::getExposureValue(bool forceStatusUpdate)
{
	return impl_->getExposureValue(forceStatusUpdate);
}


uint32_t PentaxTetherLib::registerExposureValueChangedCallback(const std::function<void(double)>& callback)
{
	return impl_->registerExposureValueChangedCallback(callback);
}


PentaxTetherLib::AutoFocusMode PentaxTetherLib::getAutoFocusMode(bool forceStatusUpdate)
{
	return impl_->getAutoFocusMode(forceStatusUpdate);
}


bool PentaxTetherLib::setAutoFocusMode(const PentaxTetherLib::AutoFocusMode& af_mode)
{
	return impl_->setAutoFocusMode(af_mode);
}


uint32_t PentaxTetherLib::registerAutoFocusModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusMode&)>& callback)
{
	return impl_->registerAutoFocusModeChangedCallback(callback);
}


uint32_t PentaxTetherLib::getNumberOfAutoFocusPoints()
{
	return impl_->getNumberOfAutoFocusPoints();
}


PentaxTetherLib::AutoFocusPointSelectionMode PentaxTetherLib::getAutoFocusPointSelectionMode(bool forceStatusUpdate)
{
	return impl_->getAutoFocusPointSelectionMode(forceStatusUpdate);
}


bool PentaxTetherLib::setAutoFocusPointSelectionMode(const PentaxTetherLib::AutoFocusPointSelectionMode& af_mode)
{
	return impl_->setAutoFocusPointSelectionMode(af_mode);
}


uint32_t PentaxTetherLib::registerAutoFocusPointSelectionModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusPointSelectionMode&)>& callback)
{
	return impl_->registerAutoFocusPointSelectionModeChangedCallback(callback);
}


std::vector<uint32_t> PentaxTetherLib::getSelectedAutoFocusPointIndex(bool forceStatusUpdate)
{
	return impl_->getSelectedAutoFocusPointIndex(forceStatusUpdate);
}


bool PentaxTetherLib::setSelectedAutoFocusPointIndex(const std::vector<uint32_t>& af_point_idx)
{
	return impl_->setSelectedAutoFocusPointIndex(af_point_idx);
}


uint32_t PentaxTetherLib::registerSelectedAutoFocusPointChangedCallback(const std::function<void(const std::vector<uint32_t>&)>& callback)
{
	return impl_->registerSelectedAutoFocusPointChangedCallback(callback);
}


/////////////////// Implementation Declaration


PentaxTetherLib::Impl::Impl( const PentaxTetherLib::Options& options)
	: options_( options ), polling_thread_( ([&,options]()
											{
												while (true)
												{
													std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int64_t>(options.statusMaxAge_sec * 1000)));
													if(isConnected())
													{ 
														pollStatus(false);
													}
												}
											}))
{
}



PentaxTetherLib::Impl::~Impl()
{
	polling_thread_.join();

	if (camhandle_ != nullptr)
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		pslr_disconnect(camhandle_);
		pslr_shutdown(camhandle_);
	}
}



void PentaxTetherLib::Impl::processStatusCallbacks()
{
	//! User Mode callback
	if (exposureModeCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->exposure_mode != lastStatus_->exposure_mode)
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : exposureModeCallbacks_)
			{
				callback.second(fromPSLR(static_cast<pslr_gui_exposure_mode_t>(currentStatus_->exposure_mode)));
			}
		}
	}

	//! ISO callback
	if (isoCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->current_iso != lastStatus_->current_iso)
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : isoCallbacks_)
			{
				callback.second(currentStatus_->current_iso);
			}
		}
	}

	//! Aperture callback
	if (apertureCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || fromPSLR<uint32_t>(currentStatus_->current_aperture) != fromPSLR<uint32_t>(lastStatus_->current_aperture))
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : apertureCallbacks_)
			{
				callback.second(fromPSLR<uint32_t>( currentStatus_->current_aperture ));
			}
		}
	}

	//! ShutterTime callback - TODO handle Bulb mode!
	if (shutterTimeCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || fromPSLR<uint32_t>(currentStatus_->current_shutter_speed) != fromPSLR<uint32_t>(lastStatus_->current_shutter_speed))
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : shutterTimeCallbacks_)
			{
				callback.second(fromPSLR<uint32_t>(currentStatus_->current_shutter_speed));
			}
		}
	}

	//! Exposure Compensation callback
	if (exposureCompensationCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || fromPSLR<int32_t>(currentStatus_->ec) != fromPSLR<int32_t>(lastStatus_->ec))
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : exposureCompensationCallbacks_)
			{
				callback.second(fromPSLR<int32_t>(currentStatus_->ec));
			}
		}
	}

	//! Battery callback
	if (batteryVoltageCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->battery_1 != lastStatus_->battery_1
			|| currentStatus_->battery_2 != lastStatus_->battery_2
			|| currentStatus_->battery_3 != lastStatus_->battery_3
			|| currentStatus_->battery_4 != lastStatus_->battery_4)
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : batteryVoltageCallbacks_)
			{
				callback.second(batteryStateFromPSLR(currentStatus_));
			}
		}
	}

	//! Focal Length callback
	if (focalLengthCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || fromPSLR<uint32_t>(currentStatus_->zoom) != fromPSLR<uint32_t>(lastStatus_->zoom))
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : focalLengthCallbacks_)
			{
				callback.second(fromPSLR<uint32_t>(currentStatus_->zoom));
			}
		}
	}

	//! Exposure Value callback
	if (exposureValueCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || calculateExposureValue( currentStatus_ ) != calculateExposureValue(lastStatus_) )
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : exposureValueCallbacks_)
			{
				callback.second(calculateExposureValue(currentStatus_));
			}
		}
	}

	//! Auto Focus Mode callback
	if (autoFocusModeCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->af_mode != lastStatus_->af_mode)
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : autoFocusModeCallbacks_)
			{
				callback.second(fromPSLR(static_cast<pslr_af_mode_t>(currentStatus_->af_mode)));
			}
		}
	}

	//! Auto Focus Point Selection Mode callback
	if (autoFocusPointSelectionModeCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->af_point_select != lastStatus_->af_point_select)
		{
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : autoFocusPointSelectionModeCallbacks_)
			{
				callback.second(fromPSLR(static_cast<pslr_af_point_sel_t>(currentStatus_->af_point_select), getNumberOfAutoFocusPoints()));
			}
		}
	}
	
	//! Auto Focus Point Selection callback
	if (selectedAutoFocusPointIndexCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->selected_af_point != lastStatus_->selected_af_point)
		{
			std::vector<uint32_t> afPoints = decodeAutoFocusPoints(currentStatus_->selected_af_point, getNumberOfAutoFocusPoints());
			std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
			for (const auto& callback : selectedAutoFocusPointIndexCallbacks_)
			{
				callback.second(afPoints);
			}
		}
	}

}


std::shared_ptr<pslr_status> PentaxTetherLib::Impl::pollStatus(bool forceStatusUpdate)
{
	if (isConnected())
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		time_t now;
		time(&now);

		std::lock_guard<std::recursive_mutex> statusLock(statusMutex_);
		if (forceStatusUpdate || difftime(now, statusUpdateTime_) > options_.statusMaxAge_sec)
		{
			std::shared_ptr<pslr_status> status = std::make_shared<pslr_status>();

			if (!testResult(pslr_get_status(camhandle_, status.get())))
			{
				currentStatus_.reset();
				statusUpdateTime_ = 0;
				return nullptr;
			}
			else
			{
				lastStatus_ = std::move(currentStatus_);
				currentStatus_ = std::move(status);
				statusUpdateTime_ = now;

				processStatusCallbacks();

				return currentStatus_;
			}
		}

		return currentStatus_;
	}
	return nullptr;
}



bool PentaxTetherLib::Impl::testResult(const int& result)
{
	switch( static_cast<pslr_result>(result))
	{
	case PSLR_OK:
		return true;
	case PSLR_DEVICE_ERROR:
		camhandle_ = nullptr;
		if (options_.reconnect)
		{
			connect(options_.reconnectionTimeout);
		}
		return false;
	case PSLR_SCSI_ERROR:
		return false;
	case PSLR_COMMAND_ERROR:
		return false;
	case PSLR_READ_ERROR:
		return false;
	case PSLR_NO_MEMORY:
		return false;
	case PSLR_PARAM:
		return false;
	default:
		return false;
	}
}


bool PentaxTetherLib::Impl::connect(unsigned int timeout_sec)
{
	disconnect();

	time_t startTime;
	time(&startTime);

	while (!(camhandle_ = pslr_init(NULL, NULL)))
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		time_t currentTime;
		time(&currentTime);

		if (difftime(currentTime, startTime) > timeout_sec)
		{
			break;
		}
	}

	if (camhandle_)
	{
		pslr_connect(camhandle_);

		// Inform listeners
		std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
		for (const auto& callback : connectionCallbacks_)
		{
			callback.second(true);
		}
		return true;
	}

	return false;
}



void PentaxTetherLib::Impl::disconnect()
{
	if (camhandle_ != nullptr)
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		pslr_disconnect(camhandle_);
	}

	// Inform listeners
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	for (const auto& callback : connectionCallbacks_)
	{
		callback.second(false);
	}
}


bool PentaxTetherLib::Impl::isConnected() const
{
	return camhandle_ != nullptr;
}


void PentaxTetherLib::Impl::unregisterCallback(const uint32_t& callbackIdentifier)
{
	connectionCallbacks_.erase(callbackIdentifier);
	exposureModeCallbacks_.erase(callbackIdentifier);
	isoCallbacks_.erase(callbackIdentifier);
	apertureCallbacks_.erase(callbackIdentifier);
	shutterTimeCallbacks_.erase(callbackIdentifier);
	exposureCompensationCallbacks_.erase(callbackIdentifier);
	batteryVoltageCallbacks_.erase(callbackIdentifier);
	focalLengthCallbacks_.erase(callbackIdentifier);
	exposureValueCallbacks_.erase(callbackIdentifier);
	autoFocusModeCallbacks_.erase(callbackIdentifier);
	autoFocusPointSelectionModeCallbacks_.erase(callbackIdentifier);
	selectedAutoFocusPointIndexCallbacks_.erase(callbackIdentifier);
}


std::string PentaxTetherLib::Impl::getCameraName()
{
	std::string camName = "Not connected";
	if (isConnected())
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		camName = std::string(pslr_camera_name(camhandle_));
	}
	return camName;
}


std::string PentaxTetherLib::Impl::getFirmware()
{
	std::string firmware = "?";
	if (isConnected())
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		char firmware_buffer[16];
		std::fill( &(firmware_buffer[0]), &(firmware_buffer[15]), 0);

		pslr_read_dspinfo(&camhandle_, firmware_buffer);
		firmware = std::string(firmware_buffer);
	}
	return firmware;
}


std::string PentaxTetherLib::Impl::getLensType()
{
	std::string lensName = "Not recognized";
	if (isConnected())
	{
		auto status = pollStatus(false);
		if (nullptr != status)
		{
			lensName = std::string(get_lens_name(status->lens_id1, status->lens_id2));
		}
	}
	return lensName;
}


std::vector<uint32_t> PentaxTetherLib::Impl::executeFocus()
{
	if (isConnected())
	{
        camCommunicationMutex_.lock();

        if (testResult(pslr_focus(camhandle_)))
		{
            camCommunicationMutex_.unlock();

            std::this_thread::sleep_for(std::chrono::milliseconds(300));
            
            auto status = pollStatus(true);
			if (nullptr != status)
			{
				return decodeAutoFocusPoints(status->focused_af_point, getNumberOfAutoFocusPoints());
			}
		}
        else
        {
            camCommunicationMutex_.unlock();
        }

	}
	return std::vector<uint32_t>();
}


bool PentaxTetherLib::Impl::executeDustRemoval()
{
	if (isConnected())
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		return testResult(pslr_dust_removal(camhandle_));
	}
	return false;
}


int32_t PentaxTetherLib::Impl::executeShutter()
{
	// TODO - handle bulb mode!

	constexpr static long extraMillisToWait = 500;
	constexpr static double extraFactorToWait = 0.1;
	constexpr static long maxMillisShutterTime = 30000;

	int32_t currentBufferIndex = PentaxTetherLib::InvalidBufferIndex;
	if (!isConnected())
	{
		return currentBufferIndex;
	}

	auto status_preShot = pollStatus(true);
	if (nullptr == status_preShot)
	{
		return currentBufferIndex;
	}

	if (status_preShot->exposure_mode == PSLR_GUI_EXPOSURE_MODE_B)
	{
		return currentBufferIndex; // Bulb mode not supported
	}

	bool result;
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);
		result = pslr_shutter(camhandle_);
	}

	if (result == PSLR_OK)
	{
		// assure synchronous method
		PentaxTetherLib::Rational<uint32_t> shutterTime = fromPSLR<uint32_t>(status_preShot->current_shutter_speed);
		if (shutterTime.isInvalid())
		{
			// Max shutter time - 30 sec
			std::this_thread::sleep_for(std::chrono::milliseconds( static_cast<long>( maxMillisShutterTime * extraFactorToWait + extraMillisToWait )));
		}
		else
		{
			const auto shuttertimeDuration = std::chrono::milliseconds(static_cast<int64_t>(std::ceil(shutterTime.toDouble() * 1000)));
			std::cout << "ShutterTime Millis: " << shuttertimeDuration.count() << std::endl;
			std::this_thread::sleep_for(shuttertimeDuration + std::chrono::milliseconds(extraMillisToWait));
		}

		bool isLimitedModel = false;
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);
			isLimitedModel = pslr_get_model_only_limited(camhandle_);
		}

		if (isLimitedModel)
		{
			currentBufferIndex = 0;
		}
		else
		{
			auto status_postShot = pollStatus(true);
			if (nullptr == status_postShot)
			{
				currentBufferIndex = PentaxTetherLib::InvalidBufferIndex;
			}
			else
			{
				if (status_postShot->bufmask != status_preShot->bufmask)
				{
					const uint32_t newBuffers = (status_postShot->bufmask ^ status_preShot->bufmask) & status_postShot->bufmask;

					if (newBuffers > 0)
					{
						currentBufferIndex = static_cast<uint32_t>( std::log2(newBuffers) );
						assert( pow(2, currentBufferIndex) == newBuffers );
					}
					else
					{
						currentBufferIndex = PentaxTetherLib::InvalidBufferIndex;
					}
				}
				else
				{
					currentBufferIndex = PentaxTetherLib::InvalidBufferIndex;
				}
			}
		}
	}

	return currentBufferIndex;
}



std::vector<uint8_t> PentaxTetherLib::Impl::getPreviewImage(int bufferIndex)
{
	std::vector<uint8_t> imageData;

	if (!isConnected())
	{
		return imageData;
	}

	uint8_t *imageBuffer;
	uint32_t imageSize;

	std::lock_guard<std::mutex> lock(camCommunicationMutex_);

	int result = pslr_get_buffer(camhandle_, bufferIndex, PSLR_BUF_PREVIEW, 4, &imageBuffer, &imageSize);
	if (testResult(result)) 
	{
		imageData.resize(imageSize);
		std::copy(imageBuffer, imageBuffer + imageSize, imageData.begin());
	}

	pslr_delete_buffer(camhandle_, bufferIndex);

	return imageData;
}


std::vector<uint8_t> PentaxTetherLib::Impl::getImage(int bufferIndex, ImageFormat format, JpgQuality jpgQuality, ImageResolution resolution, std::function<void(float)> progressCallback)
{
	std::vector<uint8_t> imageData;

	if (!isConnected())
	{
		return imageData;
	}

	auto status = pollStatus();
	if (nullptr == status)
	{
		return imageData;
	}

	pslr_buffer_type imageType;
	switch (format)
	{
	case IF_PEF:
		imageType = PSLR_BUF_PEF;
		break;
	case IF_DNG:
		imageType = PSLR_BUF_DNG;
		break;
	case IF_JPG:
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			imageType = pslr_get_jpeg_buffer_type(camhandle_, jpgQuality != JPEG_CURRENT_CAM_SETTING ? jpgQuality : status->jpeg_quality);
		}
		break;
	default:
		imageType = static_cast<pslr_buffer_type>(status->image_format);
	}


	std::lock_guard<std::mutex> lock(camCommunicationMutex_);

	bool result = testResult(pslr_buffer_open(camhandle_, bufferIndex, imageType, resolution != RES_CURRENT_CAM_SETTING ? resolution : status->jpeg_resolution));
	if (result)
	{
		uint32_t dataSize = pslr_buffer_get_size(camhandle_);
		imageData.resize(dataSize);
		uint32_t sumBytesRead = 0;
		uint8_t buffer[65536];
		for (uint32_t bytesRead = 0; true; sumBytesRead += bytesRead)
		{
			bytesRead = pslr_buffer_read(camhandle_, buffer, sizeof(buffer));
			if (bytesRead == 0)
			{
				break;
			}
			memcpy(&imageData[sumBytesRead], &buffer[0], bytesRead);

			if (progressCallback)
			{
				progressCallback(sumBytesRead / static_cast<float>(dataSize));
			}
		}
		pslr_buffer_close(camhandle_);

	}

	pslr_delete_buffer(camhandle_, bufferIndex);

	return std::move(imageData);
}


bool PentaxTetherLib::Impl::setFixedISO(uint32_t isoValue)
{
	auto status = pollStatus(true);
	if(nullptr != status && status->fixed_iso != isoValue)
	{
		// Note that camera might also handle invalid iso values by correcting them - however, we don't allow them here.
		auto validIsoValues = getISOSteps(false);
		if (std::find(validIsoValues.begin(), validIsoValues.end(), isoValue) != validIsoValues.end())
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult( pslr_set_iso(camhandle_, isoValue, 0/*autoisomin*/, 0/*autoisomax*/) );
		}
	}

	return false;
}



bool PentaxTetherLib::Impl::setAutoISORange(uint32_t minISOValue, uint32_t maxISOValue)
{
	auto status = pollStatus(true);
	if (nullptr != status && ( status->auto_iso_min != minISOValue || status->auto_iso_max != maxISOValue) )
	{
		// Note that camera might also handle invalid iso values by correcting them - however, we don't allow them here.
		auto validIsoValues = getISOSteps(false);
		if ((std::find(validIsoValues.begin(), validIsoValues.end(), minISOValue) != validIsoValues.end()) && 
			(std::find(validIsoValues.begin(), validIsoValues.end(), maxISOValue) != validIsoValues.end()))
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult(pslr_set_iso(camhandle_, 0/*fixediso*/, minISOValue, maxISOValue));
		}
	}

	return false;
}


uint32_t PentaxTetherLib::Impl::getISO(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return 0;
	}
	else
	{
		return status->current_iso;
	}
}


PentaxTetherLib::ISOSettings PentaxTetherLib::Impl::getISOSettings(bool forceStatusUpdate)
{
	PentaxTetherLib::ISOSettings isoSettings;

	auto status = pollStatus(forceStatusUpdate);
	if (nullptr != status)
	{
		isoSettings.fixedISOValue = status->fixed_iso;
		isoSettings.autoMaximumISOValue = status->auto_iso_min;
		isoSettings.autoMinimumISOValue = status->auto_iso_max;
	}

	return isoSettings;
}


bool PentaxTetherLib::Impl::setAperture(const PentaxTetherLib::Rational<uint32_t>& apertureValue)
{
	auto status = pollStatus(true);
	if (nullptr != status && fromPSLR<uint32_t>(status->current_aperture) != apertureValue)
	{
		auto validApertureValues = getApertureSteps(false);
		if (std::find(validApertureValues.begin(), validApertureValues.end(), apertureValue) != validApertureValues.end())
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult(pslr_set_aperture(camhandle_, toPSLR( apertureValue )));
		}
	}

	return false;
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::Impl::getAperture(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::Rational<uint32_t>();
	}
	else
	{
		return fromPSLR<uint32_t>(status->current_aperture);
	}
}


bool PentaxTetherLib::Impl::setShutterTime(const PentaxTetherLib::Rational<uint32_t>& shutterTime)
{
	// TODO handle bulb mode!
	auto status = pollStatus(true);
	if (nullptr != status && fromPSLR<uint32_t>(status->current_shutter_speed) != shutterTime)
	{
		auto validShutterTimes = getShutterTimeSteps(false);
		if (std::find(validShutterTimes.begin(), validShutterTimes.end(), shutterTime) != validShutterTimes.end())
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult(pslr_set_shutter(camhandle_, toPSLR(shutterTime)));
		}
	}

	return false;
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::Impl::getShutterTime(bool forceStatusUpdate)
{
	// TODO handle bulb mode!
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::Rational<uint32_t>();
	}
	else
	{
		return fromPSLR<uint32_t>(status->current_shutter_speed);
	}
}


bool PentaxTetherLib::Impl::setExposureCompensation(const PentaxTetherLib::Rational<int32_t>& ecValue)
{
	auto status = pollStatus(true);
	if (nullptr != status && fromPSLR<int32_t>(status->ec) != ecValue)
	{
		auto validExposureCompensations = getExposureCompensationSteps(false);
		if (std::find(validExposureCompensations.begin(), validExposureCompensations.end(), ecValue) != validExposureCompensations.end())
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult(pslr_set_ec(camhandle_, toPSLR(ecValue)));
		}
	}

	return false;
}


PentaxTetherLib::Rational<int32_t> PentaxTetherLib::Impl::getExposureCompensation(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::Rational<int32_t>();
	}
	else
	{
		return fromPSLR<int32_t>(status->ec);
	}
}



PentaxTetherLib::ExposureMode PentaxTetherLib::Impl::getExposureMode(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::EXPOSURE_MODE_INVALID;
	}
	else
	{
		return fromPSLR( static_cast<pslr_gui_exposure_mode_t>( status->exposure_mode ));
	}
}


std::vector<float> PentaxTetherLib::Impl::getBatteryVoltage(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<float>();
	}
	else
	{
		return batteryStateFromPSLR(status);
	}
}


PentaxTetherLib::Rational<uint32_t> PentaxTetherLib::Impl::getFocalLength(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::Rational<uint32_t>();
	}
	else
	{
		return fromPSLR<uint32_t>(status->zoom);
	}
}


double PentaxTetherLib::Impl::getExposureValue(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return 0.0;
	}
	else
	{
		return PentaxTetherLib::Impl::calculateExposureValue(status);
	}
}



bool PentaxTetherLib::Impl::setAutoFocusMode(const PentaxTetherLib::AutoFocusMode& af_mode)
{
	auto status = pollStatus(true);
	if (nullptr != status && fromPSLR(static_cast<pslr_af_mode_t>(status->af_mode)) != af_mode)
	{
		switch (af_mode)
		{
		case AutoFocusMode::AF_MODE_AF_S:
		case AutoFocusMode::AF_MODE_AF_C:
		case AutoFocusMode::AF_MODE_AF_A:
		{
			std::lock_guard<std::mutex> lock(camCommunicationMutex_);

			return testResult(pslr_set_af_mode(camhandle_, toPSLR(af_mode)));
		}
		default:
			return false;
		}
	}

	return false;
}


PentaxTetherLib::AutoFocusMode PentaxTetherLib::Impl::getAutoFocusMode(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::AF_MODE_INVALID;
	}
	else
	{
		return fromPSLR(static_cast<pslr_af_mode_t>(status->af_mode));
	}
}


uint32_t PentaxTetherLib::Impl::getNumberOfAutoFocusPoints()
{
	if (isConnected())
	{
		return pslr_get_model_af_point_num(camhandle_);
	}
	else
	{
		return 0;
	}
}


PentaxTetherLib::AutoFocusPointSelectionMode PentaxTetherLib::Impl::getAutoFocusPointSelectionMode(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return PentaxTetherLib::AF_POINT_SELECTION_INVALID;
	}
	else
	{
		return fromPSLR(static_cast<pslr_af_point_sel_t>(status->af_point_select), getNumberOfAutoFocusPoints());
	}
}



bool PentaxTetherLib::Impl::setAutoFocusPointSelectionMode(const PentaxTetherLib::AutoFocusPointSelectionMode& af_point_mode)
{
	auto status = pollStatus(true);
	if (nullptr != status && fromPSLR(static_cast<pslr_af_point_sel_t>(status->af_point_select), getNumberOfAutoFocusPoints()) != af_point_mode)
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		return testResult(pslr_set_af_point_sel(camhandle_, toPSLR(af_point_mode, getNumberOfAutoFocusPoints())));
	}

	return false;
}


std::vector<uint32_t> PentaxTetherLib::Impl::getSelectedAutoFocusPointIndex(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<uint32_t>();
	}
	else
	{
		return decodeAutoFocusPoints( status->selected_af_point, getNumberOfAutoFocusPoints() );
	}
}


bool PentaxTetherLib::Impl::setSelectedAutoFocusPointIndex(const std::vector<uint32_t>& af_point_idx)
{
	uint32_t encodedAFPoints = encodeAutoFocusPoints(af_point_idx, getNumberOfAutoFocusPoints());
	auto status = pollStatus(true);
	if (nullptr != status && status->selected_af_point != encodedAFPoints)
	{
		std::lock_guard<std::mutex> lock(camCommunicationMutex_);

		return testResult(pslr_select_af_point(camhandle_, encodedAFPoints));
	}

	return false;
}


std::vector<uint32_t> PentaxTetherLib::Impl::getISOSteps(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<uint32_t>();
	}
	
	std::vector<uint32_t> isoTable;
	if (status->custom_sensitivity_steps == PSLR_CUSTOM_SENSITIVITY_STEPS_1EV)
	{
		isoTable = {
			100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400 };

	}
	else if (status->custom_ev_steps == PSLR_CUSTOM_EV_STEPS_1_2)
	{
		isoTable = {
			100, 140, 200, 280, 400, 560, 800, 1100, 1600, 2200, 3200, 4500, 6400, 9000, 12800,
			18000, 25600, 36000, 51200, 72000, 102400 };
	}
	else
	{
		isoTable = {
			80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500,
			3200, 4000, 5000, 6400, 8000, 10000, 12800, 16000, 20000, 25600, 32000, 40000, 51200,
			64000, 80000, 102400 };
	}

	// Find particular ISO range wrt camera model - no need for thread protection
	uint32_t minISOOfModel = pslr_get_model_extended_iso_min(camhandle_);
	uint32_t maxISOOfModel = pslr_get_model_extended_iso_max(camhandle_);

	size_t min_iso_index = 0;
	size_t max_iso_index = isoTable.size() - 1;

	// cannot determine if base or extended iso is set.
	// use extended iso range
	for (size_t i = 0; i < isoTable.size(); ++i) 
	{
		if (isoTable.at(i) < minISOOfModel) 
		{
			min_iso_index = i + 1;
		}

		if (isoTable.at(i) <= maxISOOfModel)
		{
			max_iso_index = i;
		}
	}

	isoTable.erase(isoTable.begin() + max_iso_index + 1, isoTable.end());
	isoTable.erase(isoTable.begin(), isoTable.begin() + min_iso_index);

	return isoTable;
}



std::vector<PentaxTetherLib::Rational<uint32_t>> PentaxTetherLib::Impl::getApertureSteps(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<PentaxTetherLib::Rational<uint32_t>>();
	}

	std::vector< PentaxTetherLib::Rational<uint32_t> > apertureTable = {
		{10,10}, {11,10}, {12,10}, {14,10}, {16,10}, {17,10}, {18,10}, {20,10}, {22,10}, {24,10}, {25,10}, {28,10}, {32,10}, {35,10}, // this line is not confirmed yet
		{40,10}, {45,10}, {50,10}, {56,10}, {63,10}, {67,10}, {71,10}, {80,10}, {90,10}, {95,10}, {100,10}, {110,10}, {130,10}, {140,10},
		{160,10}, {180,10}, {190,10}, {200,10}, {220,10}, {250,10}, {280,10}, {320,10}, {360,10}, {400,10}, 
		{450,10}, {510,10}, {570,10} };  // this line is not confirmed yet

	size_t lowestApertureIndex  = apertureTable.size();
	size_t highestApertureIndex = apertureTable.size();
	for( size_t i = 0; i < apertureTable.size(); ++i )
	{
		if (status->lens_min_aperture.nom == apertureTable.at(i).nominator && 
			status->lens_min_aperture.denom == apertureTable.at(i).denominator)
		{
			lowestApertureIndex = i;
		}
		if (status->lens_max_aperture.nom == apertureTable.at(i).nominator && 
			status->lens_max_aperture.denom == apertureTable.at(i).denominator)
		{
			highestApertureIndex = i;
		}
	}

	if (lowestApertureIndex <= highestApertureIndex && highestApertureIndex < apertureTable.size())
	{
		apertureTable.erase(apertureTable.begin() + highestApertureIndex + 1, apertureTable.end());
		apertureTable.erase(apertureTable.begin(), apertureTable.begin() + lowestApertureIndex);

		return apertureTable;
	}
	else
	{
		return std::vector<PentaxTetherLib::Rational<uint32_t>>();
	}
}


std::vector<PentaxTetherLib::Rational<uint32_t>> PentaxTetherLib::Impl::getShutterTimeSteps(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<PentaxTetherLib::Rational<uint32_t>>();
	}

	std::vector< PentaxTetherLib::Rational<uint32_t> > exposureTimeTable;
	if (status->custom_ev_steps == PSLR_CUSTOM_EV_STEPS_1_2) 
	{
		exposureTimeTable = {
			{ 30, 1 },{ 20, 1 },{ 15, 1 },{ 10, 1 },{ 8 , 1 },{ 6 , 1 },
			{ 4 , 1 },{ 3 , 1 },{ 2 , 1 },{ 15, 10 },{ 1 , 1 },
			{ 7 , 10 },{ 5 , 10 },{ 3 , 10 },{ 1 , 4 },{ 1 , 6 },
			{ 1 , 8 },{ 1 , 10 },{ 1 , 15 },{ 1 , 20 },{ 1 , 30 },
			{ 1 , 45 },{ 1 , 60 },{ 1 , 90 },{ 1 , 125 },{ 1 , 180 },{ 1 , 250 },
			{ 1 , 350 },{ 1 , 500 },{ 1 , 750 },{ 1 , 1000 },{ 1 , 1500 },
			{ 1 , 2000 },{ 1 , 3000 },{ 1 , 4000 },{ 1, 6400 },{ 1, 8000 }
		};
	}
	else 
	{
		exposureTimeTable = {
			{ 30, 1 },{ 25, 1 },{ 20, 1 },{ 15, 1 },{ 13, 1 },{ 10, 1 },{ 8 , 1 },{ 6 , 1 },
			{ 5 , 1 },{ 4 , 1 },{ 3 , 1 },{ 25, 10 },{ 2 , 1 },{ 16, 10 },{ 13, 10 },{ 1 , 1 },
			{ 8 , 10 },{ 6 , 10 },{ 5 , 10 },{ 4 , 10 },{ 3 , 10 },{ 1 , 4 },{ 1 , 5 },{ 1 , 6 },
			{ 1 , 8 },{ 1 , 10 },{ 1 , 13 },{ 1 , 15 },{ 1 , 20 },{ 1 , 25 },{ 1 , 30 },{ 1 , 40 },
			{ 1 , 50 },{ 1 , 60 },{ 1 , 80 },{ 1 , 100 },{ 1 , 125 },{ 1 , 160 },{ 1 , 200 },{ 1 , 250 },
			{ 1 , 320 },{ 1 , 400 },{ 1 , 500 },{ 1 , 640 },{ 1 , 800 },{ 1 , 1000 },{ 1 , 1250 },{ 1 , 1600 },
			{ 1 , 2000 },{ 1 , 2500 },{ 1 , 3200 },{ 1 , 4000 },{ 1 , 5000 },{ 1, 6400 },{ 1, 8000 }
		};
	}

	return exposureTimeTable;
}



std::vector<PentaxTetherLib::Rational<int32_t>> PentaxTetherLib::Impl::getExposureCompensationSteps(bool forceStatusUpdate)
{
	auto status = pollStatus(forceStatusUpdate);
	if (nullptr == status)
	{
		return std::vector<PentaxTetherLib::Rational<int32_t>>();
	}

	// It seems that all K-models have EC between -5 .. +5

	std::vector< PentaxTetherLib::Rational<int32_t> > exposureCompensationTable;
	if (status->custom_ev_steps == PSLR_CUSTOM_EV_STEPS_1_2) 
	{
		exposureCompensationTable = {
			{ -50, 10 }, { -45, 10 }, { -40, 10 }, { -35, 10 }, { -30, 10}, { -25, 10 }, { -20, 10 }, { -15, 10 }, { -10, 10 }, { -5, 10 }, 
			{ 0, 10 }, { 5, 10 }, { 10, 10 }, { 15, 10 }, { 20, 10 }, { 30, 10 }, { 35, 10 }, { 40, 10 }, { 45, 10 }, { 50, 10 } };
	}
	else 
	{
		exposureCompensationTable = {
			{-50, 10}, {-47, 10}, {-43, 10}, {-40, 10}, {-37, 10}, {-33, 10}, {-30, 10}, {-27, 10}, {-23, 10}, {-20, 10}, 
			{-17, 10}, {-13, 10}, {-10, 10}, {-7, 10}, {-3, 10}, {0, 10}, {3, 10}, {7, 10}, {10, 10}, {13, 10}, {17, 10}, 
			{20, 10}, {23, 10}, {27, 10}, {30, 10}, {33, 10}, {37, 10}, {40, 10}, {43, 10}, {47, 10}, {50, 10} };
	}

	return exposureCompensationTable;
}



uint32_t PentaxTetherLib::Impl::registerConnectionChangedCallback(const std::function<void(bool)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	connectionCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerExposureModeChangedCallback(const std::function<void(const PentaxTetherLib::ExposureMode&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	exposureModeCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerISOChangedCallback(const std::function<void(uint32_t)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	isoCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerApertureChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	apertureCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerShutterTimeChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	shutterTimeCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerExposureCompensationChangedCallback(const std::function<void(const PentaxTetherLib::Rational<int32_t>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	exposureCompensationCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerBatteryVoltageChangedCallback(const std::function<void(const std::vector<float>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	batteryVoltageCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerFocalLengthChangedCallback(const std::function<void(const PentaxTetherLib::Rational<uint32_t>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	focalLengthCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerExposureValueChangedCallback(const std::function<void(double)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	exposureValueCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerAutoFocusModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusMode&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	autoFocusModeCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerAutoFocusPointSelectionModeChangedCallback(const std::function<void(const PentaxTetherLib::AutoFocusPointSelectionMode&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	autoFocusPointSelectionModeCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerSelectedAutoFocusPointChangedCallback(const std::function<void(const std::vector<uint32_t>&)>& callback)
{
	std::lock_guard<std::recursive_mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	selectedAutoFocusPointIndexCallbacks_.insert({ id, callback });
	return id;
}


template< typename T>
PentaxTetherLib::Rational<T> PentaxTetherLib::Impl::fromPSLR(const pslr_rational_t& r)
{
	return PentaxTetherLib::Rational<T>(r.nom, r.denom);
}


template< typename T>
pslr_rational_t PentaxTetherLib::Impl::toPSLR(const PentaxTetherLib::Rational<T>& r)
{
	pslr_rational_t p;
	p.nom = r.nominator;
	p.denom = r.denominator;
	return p;
}


PentaxTetherLib::ExposureMode PentaxTetherLib::Impl::fromPSLR(const pslr_gui_exposure_mode_t& e)
{
	return static_cast<PentaxTetherLib::ExposureMode>(e);
}


pslr_gui_exposure_mode_t PentaxTetherLib::Impl::toPSLR(const PentaxTetherLib::ExposureMode& e)
{
	return static_cast<pslr_gui_exposure_mode_t>(e);
}


PentaxTetherLib::AutoFocusMode PentaxTetherLib::Impl::fromPSLR(const pslr_af_mode_t& e)
{
	return static_cast<PentaxTetherLib::AutoFocusMode>(e);
}


pslr_af_mode_t PentaxTetherLib::Impl::toPSLR(const PentaxTetherLib::AutoFocusMode& e)
{
	return static_cast<pslr_af_mode_t>(e);
}


PentaxTetherLib::AutoFocusPointSelectionMode PentaxTetherLib::Impl::fromPSLR(const pslr_af_point_sel_t& e, const uint32_t& numberOfAFPoints)
{
	switch (numberOfAFPoints)
	{
	case 11:
		switch (static_cast<uint32_t>(e))
		{
		case 0:
			return AF_POINT_SELECTION_AUTO_5;
		case 1:
			return AF_POINT_SELECTION_SELECT_1;
		case 2:
			return AF_POINT_SELECTION_SPOT;
		case 3:
			return AF_POINT_SELECTION_AUTO_11;
		}
		break;
	case 27:
		return static_cast<PentaxTetherLib::AutoFocusPointSelectionMode>(e);
		break;
	default:
		return AF_POINT_SELECTION_INVALID;
	}
}


pslr_af_point_sel_t PentaxTetherLib::Impl::toPSLR(const PentaxTetherLib::AutoFocusPointSelectionMode& e, const uint32_t& numberOfAFPoints)
{
	switch (numberOfAFPoints)
	{
	case 11:
		switch (e)
		{
		case AF_POINT_SELECTION_AUTO_5:
			return static_cast<pslr_af_point_sel_t>(0);
		case AF_POINT_SELECTION_SELECT_1:
			return static_cast<pslr_af_point_sel_t>(1);
		case AF_POINT_SELECTION_SPOT:
			return static_cast<pslr_af_point_sel_t>(2);
		case AF_POINT_SELECTION_AUTO_11:
			return static_cast<pslr_af_point_sel_t>(3);
		}
		break;
	case 27:
		return static_cast<pslr_af_point_sel_t>(e);
		break;
	default:
		return PSLR_AF_POINT_SEL_SPOT;
	}

	return static_cast<pslr_af_point_sel_t>(e);
}


std::vector<uint32_t> PentaxTetherLib::Impl::decodeAutoFocusPoints(const uint32_t& autoFocusFlagList, const uint32_t& numberOfAFPoints)
{
	// Indices will be ordered, such that top left af point has index 0, and indices are increasing in row-major form
	std::vector<uint32_t> focusPoints;

	switch (numberOfAFPoints)
	{
	case 11: // idx scheme based on PSLR lib layout
	{
		for (uint32_t i = 0; i < numberOfAFPoints; ++i)
		{
			if ((autoFocusFlagList & (1 << i)) > 0)
			{
				focusPoints.push_back(i);
			}
		}
	}
	break;
	case 27: // idx scheme based on K3 layout
	{
		for (uint32_t i = 0; i < numberOfAFPoints; ++i)
		{
			if ((autoFocusFlagList & (1 << i)) > 0)
			{
				uint32_t idx = 0;
				if (i < 27 && i >= 17)
				{
					idx = 26 - i;
				}

				if (i == 0)
				{
					idx = 16;
				}

				if (i == 1)
				{
					idx = 10;
				}

				if (i <= 16 && i >= 2)
				{
					idx = 28 - i;
				}

				focusPoints.push_back(idx);
			}
		}
	}
	break;
	default:
		break;
	}

	return focusPoints;
}



uint32_t PentaxTetherLib::Impl::encodeAutoFocusPoints(const std::vector<uint32_t>& af_point_indices, const uint32_t& numberOfAFPoints)
{
	// Indices will be ordered, such that top left af point has index 0, and indices are increasing in row-major form
	uint32_t focusPointFlag = 0;

	switch (numberOfAFPoints)
	{
	case 11: // idx scheme based on PSLR lib layout
	{
		for (auto af_point : af_point_indices )
		{
			focusPointFlag = focusPointFlag | (1 << af_point);
		}
	}
	break;
	case 27: // idx scheme based on K3 layout
	{
		for (auto af_point : af_point_indices)
		{
			if (af_point <= 9)
			{
				focusPointFlag = focusPointFlag | (1 << (26 - af_point));
			}

			if (af_point == 10)
			{
				focusPointFlag = focusPointFlag | (1 << 1);
			}

			if (af_point == 16)
			{
				focusPointFlag = focusPointFlag | (1 << 0);
			}

			if (af_point >= 11 && af_point != 16)
			{
				focusPointFlag = focusPointFlag | (1 << (28 - af_point));
			}
		}
	}
	break;
	default:
		break;
	}

	return focusPointFlag;
}



std::vector<float> PentaxTetherLib::Impl::batteryStateFromPSLR(const std::shared_ptr<pslr_status>& status)
{
	std::vector<float> batteryVoltages;
	if (status->battery_1 > 0)
	{
		batteryVoltages.push_back(0.01f * status->battery_1);
	}
	if (status->battery_2 > 0)
	{
		batteryVoltages.push_back(0.01f * status->battery_2);
	}
	if (status->battery_3 > 0)
	{
		batteryVoltages.push_back(0.01f * status->battery_3);
	}
	if (status->battery_4 > 0)
	{
		batteryVoltages.push_back(0.01f * status->battery_4);
	}

	return batteryVoltages;
}


double PentaxTetherLib::Impl::calculateExposureValue(const std::shared_ptr<pslr_status>& status)
{
	// applied version: EV = log2( f^2 / t)
	// alternative:     EV = log2( (f^2 / t) * (100 / ISO) )
	if (status != nullptr)
	{
		const double f_sq = pow(fromPSLR<uint32_t>(status->current_aperture).toDouble(), 2.0);
		const double t = fromPSLR<uint32_t>(status->current_shutter_speed).toDouble();

		if (t > 1e-6)
		{
			return std::log2(f_sq/t);
		}
	}

	return 0;
}
