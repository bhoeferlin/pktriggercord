#include "PentaxTetherLib.h"

extern "C"
{
	bool debug = false;
	bool warnings = false;

#include "../pslr.h"
}

#include <time.h>
#include <iostream>
#include <assert.h>
#include <time.h>
#include <map>




int main()
{
	PentaxTetherLib::Options defaultOptions;
	PentaxTetherLib tetherLib( defaultOptions );
	if (tetherLib.connect(30))
	{
		std::cout << "Connected with: " << tetherLib.getCameraName() << std::endl;
		std::cout << "ISO: " << tetherLib.getISO() << std::endl;

		tetherLib.setISO(100);
		std::this_thread::sleep_for(std::chrono::seconds(2));
		std::cout << "ISO: " << tetherLib.getISO() << std::endl;

		if (tetherLib.executeFocus())
		{
			std::cout << "Focus done!" << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(2));
			uint32_t bufferIndex = tetherLib.executeShutter();
			std::this_thread::sleep_for(std::chrono::seconds(2));
			tetherLib.getImage(bufferIndex);
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

	bool executeFocus();
	uint32_t executeShutter();

	std::vector<uint8_t> getImage(int bufferIndex, ImageFormat format, JpgQuality jpgQuality, ImageResolution resolution, std::function<void(float)> progressCallback);

	uint32_t getISO(bool forceStatusUpdate);
	bool setISO(uint32_t isoValue);
	std::vector<uint32_t> getISOSteps(bool forceStatusUpdate = false);
	uint32_t registerISOChangedCallback(const std::function<void(uint32_t)>& callback);


	void unregisterCallback(const uint32_t& callbackIdentifier);

private:

	void processStatusCallbacks();


	PentaxTetherLib::Options options_;

	void* camhandle_{ nullptr };

	std::mutex statusMutex_;
	std::shared_ptr<pslr_status> currentStatus_{ nullptr };
	std::shared_ptr<pslr_status> lastStatus_{ nullptr };
	time_t statusUpdateTime_{ 0 };

	// callbacks
	std::mutex callbackMutex_;
	uint32_t nextCallbackIdentifier_{0};
	std::map< uint32_t, std::function<void(bool)> > connectionCallbacks_;
	std::map< uint32_t, std::function<void(uint32_t)> > isoCallbacks_;

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


bool PentaxTetherLib::executeFocus()
{
	return impl_->executeFocus();
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


uint32_t PentaxTetherLib::getISO(bool forceStatusUpdate)
{
	return impl_->getISO(forceStatusUpdate);
}


bool PentaxTetherLib::setISO(uint32_t isoValue)
{
	return impl_->setISO(isoValue);
}


std::vector<uint32_t> PentaxTetherLib::getISOSteps()
{
	return impl_->getISOSteps(false);
}


uint32_t PentaxTetherLib::registerISOChangedCallback(const std::function<void(uint32_t)>& callback)
{
	return impl_->registerISOChangedCallback(callback);
}






/////////////////// Implementation Declaration


PentaxTetherLib::Impl::Impl( const PentaxTetherLib::Options& options)
	: options_( options )
{
}



PentaxTetherLib::Impl::~Impl()
{
	if (camhandle_ != nullptr)
	{
		pslr_disconnect(camhandle_);
		pslr_shutdown(camhandle_);
	}
}



void PentaxTetherLib::Impl::processStatusCallbacks()
{
	//! ISO callback
	if (isoCallbacks_.size() > 0 && currentStatus_ != nullptr)
	{
		if (lastStatus_ == nullptr || currentStatus_->fixed_iso != lastStatus_->fixed_iso)
		{
			std::lock_guard<std::mutex> lock(callbackMutex_);
			for (const auto& callback : isoCallbacks_)
			{
				callback.second(currentStatus_->fixed_iso);
			}
		}
	}
}



std::shared_ptr<pslr_status> PentaxTetherLib::Impl::pollStatus(bool forceStatusUpdate)
{
	if (isConnected())
	{
		time_t now;
		time(&now);

		std::lock_guard<std::mutex> lock(statusMutex_);
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
		if (options_.reconnect_)
		{
			connect(options_.reconnectionTimeout_);
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
		std::lock_guard<std::mutex> lock(callbackMutex_);
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
		pslr_disconnect(camhandle_);
	}

	// Inform listeners
	std::lock_guard<std::mutex> lock(callbackMutex_);
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
	isoCallbacks_.erase(callbackIdentifier);
}



std::string PentaxTetherLib::Impl::getCameraName()
{
	std::string camName = "Not connected";
	if (isConnected())
	{
		camName = std::string(pslr_camera_name(camhandle_));
	}
	return camName;
}



bool PentaxTetherLib::Impl::executeFocus()
{
	if (!isConnected())
	{
		return false;
	}
	return testResult(pslr_focus(camhandle_));
}



uint32_t PentaxTetherLib::Impl::executeShutter()
{
	uint32_t currentBufferIndex = PentaxTetherLib::InvalidBufferIndex;
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

	bool result = pslr_shutter(camhandle_);
	if (result == PSLR_OK)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));

		if (pslr_get_model_only_limited(camhandle_))
		{
			currentBufferIndex = 0;
		}
		else
		{
			auto status_postShot = pollStatus(true);
			if (nullptr == status_postShot)
			{
				return currentBufferIndex;
			}			
			
			// TODO shape of unclear or maybe different between camera models? 

			//assert(status_postShot->bufmask != status_preShot->bufmask);

			uint32_t newBuffers = status_postShot->bufmask; 
			//uint32_t newBuffers = (status_postShot->bufmask ^ status_preShot->bufmask) & status_postShot->bufmask;
															//assert(newBuffers > 0);

			int tmpBufferIndex = PentaxTetherLib::InvalidBufferIndex;
			for (; tmpBufferIndex >= 0; --tmpBufferIndex)
			{
				if (newBuffers & (1 << tmpBufferIndex))
				{
					break;
				}
			}

			if (tmpBufferIndex >= 0)
			{
				currentBufferIndex = tmpBufferIndex;
			}
		}
	}

	return currentBufferIndex;
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
		imageType = pslr_get_jpeg_buffer_type(camhandle_, jpgQuality != JPEG_CURRENT_CAM_SETTING ? jpgQuality : status->jpeg_quality);
		break;
	default:
		imageType = static_cast<pslr_buffer_type>(status->image_format);
	}

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

	return std::move(imageData);
}


bool PentaxTetherLib::Impl::setISO(uint32_t isoValue)
{
	auto status = pollStatus(true);
	if(nullptr != status && status->fixed_iso != isoValue)
	{
		auto validIsoValues = getISOSteps(false);
		if (std::find(validIsoValues.begin(), validIsoValues.end(), isoValue) != validIsoValues.end())
		{
			return testResult( pslr_set_iso(camhandle_, isoValue, 0, 0) );
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
		return status->fixed_iso;
	}
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
		const uint32_t isoTableData[] = {
			80, 100, 125, 160, 200, 250, 320, 400, 500, 640, 800, 1000, 1250, 1600, 2000, 2500,
			3200, 4000, 5000, 6400, 8000, 10000, 12800, 16000, 20000, 25600, 32000, 40000, 51200, 
			64000, 80000, 102400 };
		isoTable = std::vector<uint32_t>(isoTableData, isoTableData + sizeof(isoTableData) / sizeof(isoTableData[0]));
	}
	else if (status->custom_ev_steps == PSLR_CUSTOM_EV_STEPS_1_2)
	{
		const uint32_t isoTableData[] = {
			100, 140, 200, 280, 400, 560, 800, 1100, 1600, 2200, 3200, 4500, 6400, 9000, 12800,
			18000, 25600, 36000, 51200, 72000, 102400 };
		isoTable = std::vector<uint32_t>(isoTableData, isoTableData + sizeof(isoTableData) / sizeof(isoTableData[0]));
	}
	else
	{
		const uint32_t isoTableData[] = {
			100, 200, 400, 800, 1600, 3200, 6400, 12800, 25600, 51200, 102400 };
		isoTable = std::vector<uint32_t>(isoTableData, isoTableData + sizeof(isoTableData) / sizeof(isoTableData[0]));
	}

	// Find particular ISO range wrt camera model
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


uint32_t PentaxTetherLib::Impl::registerISOChangedCallback(const std::function<void(uint32_t)>& callback)
{
	std::lock_guard<std::mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	isoCallbacks_.insert({ id, callback });
	return id;
}


uint32_t PentaxTetherLib::Impl::registerConnectionChangedCallback(const std::function<void(bool)>& callback)
{
	std::lock_guard<std::mutex> lock(callbackMutex_);
	uint32_t id = (++nextCallbackIdentifier_);
	connectionCallbacks_.insert({ id, callback });
	return id;
}




