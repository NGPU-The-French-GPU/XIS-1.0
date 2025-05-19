#pragma once

#include <memory>

namespace XIS {

// Déclarations anticipées
class IRenderer;

/**
 * @brief Étape de réduction de résolution (downsampling) dans le pipeline
 * 
 * Cette classe implémente l'étape optionnelle de réduction de résolution
 * qui peut être utilisée pour réduire le bruit avant l'upscaling ou 
 * pour optimiser les performances.
 */
class DownsampleStage {
public:
    /**
     * @brief Constructeur
     * 
     * @param renderer Renderer à utiliser
     */
    explicit DownsampleStage(std::shared_ptr<IRenderer> renderer);
    ~DownsampleStage();

    /**
     * @brief Initialise l'étape de downsampling
     * 
     * @return true si l'initialisation réussit, false sinon
     */
    bool Initialize();

    /**
     * @brief Traite une frame en réduisant sa résolution
     * 
     * @param inputTexture Texture d'entrée
     * @param outputTexture Texture de sortie (résolution réduite)
     * @param factor Facteur de réduction optionnel (0 = utiliser la valeur par défaut)
     * @return true si le traitement réussit, false sinon
     */
    bool Process(void* inputTexture, void* outputTexture, float factor = 0.0f);

    /**
     * @brief Configure le facteur de réduction de résolution
     * 
     * @param factor Facteur de réduction (0.25 = 1/4 de la résolution originale)
     */
    void SetDownsampleFactor(float factor);

private:
    std::shared_ptr<IRenderer> m_renderer;
    
    // Ressources des shaders
    void* m_downsampleShader;
    void* m_constantBuffer;
    
    // Paramètres
    struct DownsampleParams {
        float downsampleFactor;  // Facteur de réduction
        float preserveDetail;    // Facteur de préservation des détails
        float threshold;         // Seuil pour la détection des contours
        float reserved;          // Pour alignement
    };
    
    DownsampleParams m_params;
    
    // Méthodes d'initialisation des ressources
    bool CreateShaderResources();
    void UpdateConstantBuffer();
};

} // namespace XIS