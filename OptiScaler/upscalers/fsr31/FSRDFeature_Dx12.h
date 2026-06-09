#pragma once
#include "FSR31Feature_Dx12.h"
#include "shaders/fsrd_preprocess/FSRDPreprocessor_Dx12.h"
#include <DirectXMath.h>

/**
 * @brief Unfied denoiser-upscaler utilising AMD FSR Ray Regeneration and Super Resolution with
 * DLSS-RR inputs. Extends FSR 3.1+ upscaler implementation.
 */
class FSRDFeatureDx12 : public FSR31FeatureDx12
{
  public:
    using FSRDConvDesc = FSRDPreprocessor_Dx12::ConversionDesc;

    FSRDFeatureDx12(uint32_t InHandleId, NVSDK_NGX_Parameter* InParameters);

    ~FSRDFeatureDx12();

    feature_version Version() override { return FSR31FeatureDx12::Version(); }

    std::string Name() const override { return FSR31FeatureDx12::Name(); }

    bool Evaluate(ID3D12GraphicsCommandList* InCommandList, NVSDK_NGX_Parameter* InParameters) override;

  private:

    union DenoiserConfiguration
    {
        static constexpr int kFirstKey = (int)FFX_API_CONFIGURE_DENOISER_KEY_CROSS_BILATERAL_NORMAL_STRENGTH;
        static constexpr int kLastKey = (int)FFX_API_CONFIGURE_DENOISER_KEY_DISOCCLUSION_THRESHOLD;
        static constexpr uint32_t kCount = (uint32_t)(kLastKey - kFirstKey + 1);
        static constexpr uint32_t kKnownKeyCount = 6u;

        static_assert(kFirstKey <= kLastKey, "Unexpected FidelityFX denoiser key ordering");
        static_assert(kCount == kKnownKeyCount,
                      "FidelityFX denoiser key count changed; update DenoiserConfiguration member order");

        // Ordered by FfxApiConfigureDenoiserKey. FidelityFX denoiser keys are not local
        // array indices, so keep conversion centralized and symmetric. This prevents
        // SetDefaultConfiguration() from ever querying/configuring a non-existent key 0.
        // If a newer SDK adds/removes denoiser keys, the static_assert above forces this
        // struct and the UI defaults to be reviewed together.
        struct
        {
            float m_CrossBilateralNormalStrength;
            float m_StabilityBias;
            float m_MaxRadiance;
            float m_RadianceClipStdK;
            float m_GaussianKernelRelaxation;
            float m_DisocclusionThreshold;
        };

        float AsArray[kCount];

        static int GetKeyIndex(FfxApiConfigureDenoiserKey key) 
        {
            return std::clamp((int)key, kFirstKey, kLastKey) - kFirstKey;
        }

        static FfxApiConfigureDenoiserKey GetIndexKey(int index)
        {
            index = std::clamp(index, 0, (int)DenoiserConfiguration::kCount - 1);
            return static_cast<FfxApiConfigureDenoiserKey>(index + kFirstKey);
        }

        float& GetMember(int index) { return AsArray[std::clamp(index, 0, (int)DenoiserConfiguration::kCount - 1)]; }

        float& GetMember(FfxApiConfigureDenoiserKey key) { return AsArray[GetKeyIndex(key)]; }
    };

    ffxContext _pDenoiserCtx = nullptr;
    ffxCreateContextDescDenoiser _denoiserCtxDesc {};
    DenoiserConfiguration _denoiserSettings {};
    bool _isMode2 = false;

    static bool s_isHWDepth;
    static bool s_isRoughnessPacked;

    FSRDConvDesc _convDesc {};
    DirectX::XMFLOAT3 _lastCamPos {}; // Last world space camera position

    // Matrices
    DirectX::XMMATRIX _invViewMatrix = DirectX::XMMatrixIdentity();   // Camera rotation and translation
    DirectX::XMMATRIX _viewMatrix = DirectX::XMMatrixIdentity();      // World to camera space
    DirectX::XMMATRIX _prevViewMatrix = DirectX::XMMatrixIdentity();  // Last world to camera space
    DirectX::XMMATRIX _projMatrix = DirectX::XMMatrixIdentity();      // Perspective projection matrix
    bool _isRightHanded = false;                                      // True if the camera matrix is right handed

    std::unique_ptr<FSRDPreprocessor_Dx12> FSRDConvShader;

    bool InitFSR3(const NVSDK_NGX_Parameter* InParameters) override;

    bool CreateDenoiserContext();

    bool QueryDenoiserVersions();

    void DestroyDenoiserContext();

    void UpdateSize();

    /**
     * @brief Generates FFX denoiser configuration and input buffers from DLSS-RR inputs and NGX configurations.
     * Converts and repacks resources internally.
     */
    template<typename SignalDescT>
    bool PrepareDenoiserInput(ID3D12GraphicsCommandList* InCommandList, const NVSDK_NGX_Parameter& ngxParams,
                              ffxDispatchDescDenoiser& dispatchDesc, SignalDescT& signalDesc);

    /**
     * @brief Retrieves DLSS-RR inputs to populate the inputs for the interop layer in order to generate
     FSR-RR compatible buffers.
     */
    bool PrepareDenoiseConvInput(const NVSDK_NGX_Parameter& inParams);

    /**
     * @brief Converts previously retrieved DLSS-RR resources into FSR-RR inputs.
     */
    bool ConvertDenoiserBuffers(ID3D12GraphicsCommandList* InCommandList);

    /**
     * @brief Dispatches FSR-RR denoiser converted inputs. Runs before upscaler.
     */
    bool DispatchDenoiser(ID3D12GraphicsCommandList* InCommandList, const ffxDispatchDescDenoiser& dispatchDesc);

    void SetDefaultConfiguration();

    ffxReturnCode_t SetDefaultConfiguration(FfxApiConfigureDenoiserKey key);

    ffxReturnCode_t ApplyConfiguration(FfxApiConfigureDenoiserKey key);
};