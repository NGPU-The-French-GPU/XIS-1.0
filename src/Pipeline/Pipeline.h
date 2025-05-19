#pragma once

#include <memory>
#include <vector>
#include "../Core/XISParameters.h"

namespace XIS {

// Déclarations anticipées
class IRenderer;
class DownsampleStage;
class AntiAliasingStage;
class UpscalingStage;
class SharpnessStage;
class FrameGenStage;
class BicubicUpscaler;
class FrameInterpolator;

/**
 * @brief Classe définissant le pipeline de traitement XIS
 * 
 * Cette classe gère l'exécution et la coordination des différentes étapes
 * du pipeline de traitement: downsampling optionnel, antialiasing,
 * upscaling bicubique, génération de frame et sharpness.
 */
class Pipeline {
public:
    /**
     * @brief Constructeur
     * 
     * @param renderer Renderer à utiliser
     */
    explicit Pipeline(std::shared_ptr<IRenderer> renderer);
    ~Pipeline();

    /**
     * @brief Initialise le pipeline avec la configuration spécifiée
     * 
     * @param config Configuration XIS
     * @return true si l'initialisation réussit, false sinon
     */
    bool Initialize(const XISConfig& config);

    /**
     * @brief Exécute le pipeline complet sur une frame
     * 
     * @param params Paramètres de traitement
     * @return true si le traitement réussit, false sinon
     */
    bool Execute(const XISParameters& params);

    /**
     * @brief Met à jour les paramètres d'upscaling
     * 
     * @param params Nouveaux paramètres d'upscaling
     */
    void UpdateUpscalingParameters(const UpscalingParameters& params);

    /**
     * @brief Met à jour les paramètres de génération de frames
     * 
     * @param params Nouveaux paramètres de génération de frames
     */
    void UpdateFrameGenParameters(const FrameGenParameters& params);

    /**
     * @brief Active ou désactive l'upscaling
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableUpscaling(bool enabled);

    /**
     * @brief Active ou désactive la génération de frames
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableFrameGeneration(bool enabled);

    /**
     * @brief Active ou désactive l'antialiasing
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableAntiAliasing(bool enabled);

    /**
     * @brief Active ou désactive le sharpening
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableSharpening(bool enabled);

    /**
     * @brief Obtient les statistiques de performance actuelles
     * 
     * @return Statistiques de performance
     */
    XISPerformanceStats GetPerformanceStats() const;

private:
    // Renderer
    std::shared_ptr<IRenderer> m_renderer;

    // Étapes du pipeline
    std::unique_ptr<DownsampleStage> m_downsampleStage;
    std::unique_ptr<AntiAliasingStage> m_antiAliasingStage;
    std::unique_ptr<UpscalingStage> m_upscalingStage;
    std::unique_ptr<SharpnessStage> m_sharpnessStage;
    std::unique_ptr<FrameGenStage> m_frameGenStage;

    // Algorithmes
    std::shared_ptr<BicubicUpscaler> m_bicubicUpscaler;
    std::shared_ptr<FrameInterpolator> m_frameInterpolator;

    // Configuration
    XISConfig m_config;
    
    // États d'activation
    bool m_upscalingEnabled;
    bool m_frameGenEnabled;
    bool m_antiAliasingEnabled;
    bool m_sharpnessEnabled;

    // Statistiques de performance
    XISPerformanceStats m_perfStats;
    
    // Méthodes internes
    bool InitializeStages();
    void UpdatePipelineStages();
};

} // namespace XIS