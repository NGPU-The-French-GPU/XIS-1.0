#pragma once

#include "XISConfig.h"
#include "XISIntegration.h"

namespace XIS {

/**
 * @brief Classe principale pour le système XIS d'upscaling et génération de frames
 * 
 * Cette classe expose l'API publique pour l'intégration du système dans une application.
 * Elle coordonne l'upscaling bicubique et la génération/interpolation de frames.
 */
class XISAPI {
public:
    /**
     * @brief Initialise le système XIS
     * 
     * @param config Configuration initiale du système
     * @return true si l'initialisation réussit, false sinon
     */
    static bool Initialize(const XISConfig& config);

    /**
     * @brief Termine et libère les ressources du système XIS
     */
    static void Shutdown();

    /**
     * @brief Obtient l'instance de XIS (Singleton)
     * 
     * @return L'instance unique de XISAPI
     */
    static XISAPI& GetInstance();

    /**
     * @brief Traite une frame source avec upscaling et interpolation optionnelle
     * 
     * @param sourceTexture Texture source à traiter
     * @param outputTexture Texture de sortie pour le résultat
     * @param parameters Paramètres pour le traitement de cette frame
     * @return true si le traitement a réussi, false sinon
     */
    bool ProcessFrame(void* sourceTexture, void* outputTexture, const XISParameters& parameters);

    /**
     * @brief Active ou désactive l'upscaling bicubique
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableBicubicUpscaling(bool enabled);

    /**
     * @brief Active ou désactive la génération de frames
     * 
     * @param enabled True pour activer, false pour désactiver
     */
    void EnableFrameGeneration(bool enabled);

    /**
     * @brief Configure les paramètres d'upscaling
     * 
     * @param params Paramètres d'upscaling
     */
    void ConfigureUpscaling(const UpscalingParameters& params);

    /**
     * @brief Configure les paramètres de génération de frames
     * 
     * @param params Paramètres de génération de frames
     */
    void ConfigureFrameGeneration(const FrameGenParameters& params);

    /**
     * @brief Obtient les statistiques de performance actuelles
     * 
     * @return Structure contenant les données de performance
     */
    XISPerformanceStats GetPerformanceStats() const;

private:
    XISAPI(); // Constructeur privé (Singleton)
    ~XISAPI();
    
    // Empêcher la copie
    XISAPI(const XISAPI&) = delete;
    XISAPI& operator=(const XISAPI&) = delete;

    // Implémentation interne
    class Impl;
    std::unique_ptr<Impl> m_pImpl;
    
    static XISAPI* s_pInstance;
};

} // namespace XIS