#pragma once

#include <cstdint>

namespace XIS {

// Définition des types d'API exportés
#ifdef XIS_EXPORTS
    #ifdef _WIN32
        #define XIS_API __declspec(dllexport)
    #else
        #define XIS_API __attribute__((visibility("default")))
    #endif
#else
    #ifdef _WIN32
        #define XIS_API __declspec(dllimport)
    #else
        #define XIS_API
    #endif
#endif

/**
 * @brief Mode d'upscaling disponibles
 */
enum class UpscalingMode {
    Bicubic,           // Upscaling bicubique standard
    BicubicSharp,      // Bicubique avec netteté améliorée
    BicubicAdaptive    // Bicubique avec adaptation de netteté selon le contenu
};

/**
 * @brief Mode de génération de frames
 */
enum class FrameGenMode {
    Interpolation,     // Interpolation simple entre frames
    MotionCompensated, // Interpolation avec compensation de mouvement
    Advanced           // Algorithme avancé de génération
};

/**
 * @brief Qualité de l'antialiasing
 */
enum class AAQuality {
    Off,
    Low,
    Medium,
    High
};

/**
 * @brief Configuration de l'upscaling
 */
struct UpscalingParameters {
    UpscalingMode mode = UpscalingMode::BicubicAdaptive;
    float sharpnessStrength = 0.5f;       // Force de la netteté [0.0 - 1.0]
    float edgePreservation = 0.7f;        // Conservation des contours [0.0 - 1.0]
    uint32_t outputWidth = 0;             // Largeur cible (0 = automatique)
    uint32_t outputHeight = 0;            // Hauteur cible (0 = automatique)
    bool preserveFilmGrain = false;       // Conserver le grain de film
};

/**
 * @brief Configuration de la génération de frames
 */
struct FrameGenParameters {
    FrameGenMode mode = FrameGenMode::MotionCompensated;
    uint32_t targetFrameRate = 60;        // Fréquence d'images cible
    float motionSensitivity = 0.5f;       // Sensibilité à la détection de mouvement [0.0 - 1.0]
    float artifactReduction = 0.6f;       // Réduction des artefacts [0.0 - 1.0]
    bool enableSceneChangeDetection = true; // Détection des changements de scène
};

/**
 * @brief Configuration générale du système XIS
 */
struct XISConfig {
    bool enableBicubicUpscaling = true;   // Activer l'upscaling bicubique
    bool enableFrameGeneration = true;    // Activer la génération de frames
    bool enableAntiAliasing = true;       // Activer l'antialiasing
    AAQuality aaQuality = AAQuality::Medium; // Qualité de l'antialiasing
    bool enableSharpness = true;          // Activer l'étape de netteté
    
    UpscalingParameters upscalingParams;  // Paramètres d'upscaling
    FrameGenParameters frameGenParams;    // Paramètres de génération de frames
    
    bool enableLogging = true;            // Activer la journalisation
    bool enablePerfMonitoring = true;     // Activer la surveillance des performances
    
    const char* shaderPath = nullptr;     // Chemin vers les shaders (nullptr = utiliser chemin par défaut)
};

/**
 * @brief Paramètres pour le traitement d'une frame spécifique
 */
struct XISParameters {
    void* inputTexture = nullptr;         // Texture d'entrée
    void* outputTexture = nullptr;        // Texture de sortie
    
    // Paramètres de timing pour la génération de frames
    float frameDeltaTime = 0.0f;          // Temps écoulé depuis la dernière frame
    
    // Facteurs de qualité dynamiques
    float qualityFactor = 1.0f;           // Facteur de qualité dynamique [0.0 - 1.0]
    
    // Paramètres spécifiques à DirectX
    bool isDX11 = true;                   // true pour DX11, false pour DX12
    void* deviceContext = nullptr;        // Contexte de périphérique (ID3D11DeviceContext* ou ID3D12GraphicsCommandList*)
};

/**
 * @brief Statistiques de performance
 */
struct XISPerformanceStats {
    float processingTimeMs = 0.0f;        // Temps de traitement total en ms
    float upscalingTimeMs = 0.0f;         // Temps d'upscaling en ms
    float frameGenTimeMs = 0.0f;          // Temps de génération de frame en ms
    float gpuUsagePercent = 0.0f;         // Utilisation GPU estimée
    uint32_t inputResolution[2] = {0, 0}; // Résolution d'entrée [largeur, hauteur]
    uint32_t outputResolution[2] = {0, 0}; // Résolution de sortie [largeur, hauteur]
    float outputFps = 0.0f;               // FPS estimés en sortie
};

} // namespace XIS