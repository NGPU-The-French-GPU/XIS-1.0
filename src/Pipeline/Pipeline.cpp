#include "Pipeline.h"
#include "../Core/XISContext.h"
#include "../Core/XISParameters.h"
#include "../Renderer/IRenderer.h"
#include "DownsampleStage.h"
#include "AntiAliasingStage.h"
#include "UpscalingStage.h"
#include "SharpnessStage.h"
#include "FrameGenStage.h"
#include "../Algorithms/BicubicUpscaler.h"
#include "../Algorithms/FrameInterpolator.h"
#include "../Utils/Logger.h"
#include "../Utils/PerfMonitor.h"

namespace XIS {

Pipeline::Pipeline(std::shared_ptr<IRenderer> renderer)
    : m_renderer(renderer),
      m_upscalingEnabled(true),
      m_frameGenEnabled(true),
      m_antiAliasingEnabled(true),
      m_sharpnessEnabled(true)
{
}

Pipeline::~Pipeline() = default;

bool Pipeline::Initialize(const XISConfig& config)
{
    m_config = config;
    
    // Initialiser les états d'activation en fonction de la configuration
    m_upscalingEnabled = config.enableBicubicUpscaling;
    m_frameGenEnabled = config.enableFrameGeneration;
    m_antiAliasingEnabled = config.enableAntiAliasing;
    m_sharpnessEnabled = config.enableSharpness;
    
    // Initialiser les algorithmes partagés
    m_bicubicUpscaler = std::make_shared<BicubicUpscaler>();
    if (!m_bicubicUpscaler->Initialize(m_renderer)) {
        Logger::Error("Échec de l'initialisation de BicubicUpscaler");
        return false;
    }
    
    m_frameInterpolator = std::make_shared<FrameInterpolator>();
    if (!m_frameInterpolator->Initialize(m_renderer)) {
        Logger::Error("Échec de l'initialisation de FrameInterpolator");
        return false;
    }
    
    // Initialiser les étapes du pipeline
    return InitializeStages();
}

bool Pipeline::InitializeStages()
{
    // Créer et initialiser les étapes du pipeline
    m_downsampleStage = std::make_unique<DownsampleStage>(m_renderer);
    if (!m_downsampleStage->Initialize()) {
        Logger::Error("Échec de l'initialisation de DownsampleStage");
        return false;
    }
    
    m_antiAliasingStage = std::make_unique<AntiAliasingStage>(m_renderer);
    if (!m_antiAliasingStage->Initialize(m_config.aaQuality)) {
        Logger::Error("Échec de l'initialisation de AntiAliasingStage");
        return false;
    }
    
    m_upscalingStage = std::make_unique<UpscalingStage>(m_renderer, m_bicubicUpscaler);
    if (!m_upscalingStage->Initialize(m_config.upscalingParams)) {
        Logger::Error("Échec de l'initialisation de UpscalingStage");
        return false;
    }
    
    m_sharpnessStage = std::make_unique<SharpnessStage>(m_renderer);
    if (!m_sharpnessStage->Initialize(m_config.upscalingParams.sharpnessStrength)) {
        Logger::Error("Échec de l'initialisation de SharpnessStage");
        return false;
    }
    
    m_frameGenStage = std::make_unique<FrameGenStage>(m_renderer, m_frameInterpolator);
    if (!m_frameGenStage->Initialize(m_config.frameGenParams)) {
        Logger::Error("Échec de l'initialisation de FrameGenStage");
        return false;
    }
    
    Logger::Info("Toutes les étapes du pipeline ont été initialisées avec succès");
    return true;
}

bool Pipeline::Execute(const XISParameters& params)
{
    // Démarrer le monitoring de performance
    auto perfMonitor = PerfMonitor::GetInstance();
    perfMonitor->StartFrame();
    
    // Préparer les ressources pour le pipeline
    void* currentInput = params.inputTexture;
    void* intermediateOutput = nullptr;
    void* finalOutput = params.outputTexture;
    
    // Vérifier si nous avons des étapes activées
    bool anyStageEnabled = m_upscalingEnabled || m_frameGenEnabled || 
                          m_antiAliasingEnabled || m_sharpnessEnabled;
    
    if (!anyStageEnabled) {
        // Aucune étape activée, copier directement l'entrée vers la sortie
        m_renderer->CopyResource(currentInput, finalOutput);
        return true;
    }
    
    // Créer des textures intermédiaires selon les besoins
    m_renderer->CreateIntermediateResources(params);
    
    // 1. Étape optionnelle de downsampling (pour réduire le bruit avant upscaling)
    if (m_config.upscalingParams.mode == UpscalingMode::BicubicAdaptive) {
        perfMonitor->StartStage("Downsample");
        intermediateOutput = m_renderer->GetIntermediateResource(0);
        m_downsampleStage->Process(currentInput, intermediateOutput);
        currentInput = intermediateOutput;
        perfMonitor->EndStage("Downsample");
    }
    
    // 2. Étape d'antialiasing
    if (m_antiAliasingEnabled && m_config.aaQuality != AAQuality::Off) {
        perfMonitor->StartStage("AntiAliasing");
        intermediateOutput = m_renderer->GetIntermediateResource(1);
        m_antiAliasingStage->Process(currentInput, intermediateOutput);
        currentInput = intermediateOutput;
        perfMonitor->EndStage("AntiAliasing");
    }
    
    // 3. Étape d'upscaling bicubique
    if (m_upscalingEnabled) {
        perfMonitor->StartStage("Upscaling");
        intermediateOutput = m_renderer->GetIntermediateResource(2);
        m_upscalingStage->Process(currentInput, intermediateOutput);
        currentInput = intermediateOutput;
        perfMonitor->EndStage("Upscaling");
    }
    
    // 4. Étape de génération/interpolation de frames
    if (m_frameGenEnabled) {
        perfMonitor->StartStage("FrameGen");
        intermediateOutput = m_renderer->GetIntermediateResource(3);
        m_frameGenStage->Process(currentInput, intermediateOutput, params.frameDeltaTime);
        currentInput = intermediateOutput;
        perfMonitor->EndStage("FrameGen");
    }
    
    // 5. Étape d'amélioration de la netteté
    if (m_sharpnessEnabled) {
        perfMonitor->StartStage("Sharpness");
        m_sharpnessStage->Process(currentInput, finalOutput);
        perfMonitor->EndStage("Sharpness");
    } else {
        // Copier le résultat final si l'étape de netteté est désactivée
        m_renderer->CopyResource(currentInput, finalOutput);
    }
    
    // Terminer le monitoring de performance
    perfMonitor->EndFrame();
    m_perfStats = perfMonitor->GetStats();
    
    // Nettoyer les ressources intermédiaires
    m_renderer->ReleaseIntermediateResources();
    
    return true;
}

void Pipeline::UpdateUpscalingParameters(const UpscalingParameters& params)
{
    if (m_upscalingStage) {
        m_upscalingStage->UpdateParameters(params);
    }
    
    if (m_sharpnessStage) {
        m_sharpnessStage->UpdateSharpnessStrength(params.sharpnessStrength);
    }
    
    m_config.upscalingParams = params;
}

void Pipeline::UpdateFrameGenParameters(const FrameGenParameters& params)
{
    if (m_frameGenStage) {
        m_frameGenStage->UpdateParameters(params);
    }
    
    m_config.frameGenParams = params;
}

void Pipeline::EnableUpscaling(bool enabled)
{
    m_upscalingEnabled = enabled;
}

void Pipeline::EnableFrameGeneration(bool enabled)
{
    m_frameGenEnabled = enabled;
}

void Pipeline::EnableAntiAliasing(bool enabled)
{
    m_antiAliasingEnabled = enabled;
}

void Pipeline::EnableSharpening(bool enabled)
{
    m_sharpnessEnabled = enabled;
}

XISPerformanceStats Pipeline::GetPerformanceStats() const
{
    return m_perfStats;
}

} // namespace XIS