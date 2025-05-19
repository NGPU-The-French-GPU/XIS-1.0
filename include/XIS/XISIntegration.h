#pragma once

#include "XISConfig.h"

namespace XIS {

/**
 * @brief Fonctions d'intégration pour le système XIS
 * 
 * Ces fonctions fournissent un moyen simple d'intégrer le système XIS
 * dans une application existante.
 */

/**
 * @brief Initialise le système XIS avec la configuration spécifiée
 * 
 * @param config Configuration du système
 * @return true si l'initialisation a réussi, false sinon
 */
XIS_API bool Initialize(const XISConfig& config);

/**
 * @brief Termine le système XIS et libère les ressources
 */
XIS_API void Shutdown();

/**
 * @brief Traite une frame en appliquant l'upscaling et/ou la génération de frames
 * 
 * @param params Paramètres de traitement de la frame
 * @return true si le traitement a réussi, false sinon
 */
XIS_API bool ProcessFrame(const XISParameters& params);

/**
 * @brief Met à jour les paramètres d'upscaling
 * 
 * @param params Nouveaux paramètres d'upscaling
 */
XIS_API void SetUpscalingParameters(const UpscalingParameters& params);

/**
 * @brief Met à jour les paramètres de génération de frames
 * 
 * @param params Nouveaux paramètres de génération de frames
 */
XIS_API void SetFrameGenParameters(const FrameGenParameters& params);

/**
 * @brief Obtient les statistiques de performance actuelles
 * 
 * @param stats Structure à remplir avec les statistiques
 */
XIS_API void GetPerformanceStats(XISPerformanceStats& stats);

/**
 * @brief Fonctions d'intégration spécifiques à DirectX 11
 */
namespace DX11 {
    /**
     * @brief Initialise XIS avec un périphérique DirectX 11
     * 
     * @param device Pointeur vers le périphérique ID3D11Device
     * @param deviceContext Pointeur vers le contexte ID3D11DeviceContext
     * @param config Configuration du système
     * @return true si l'initialisation a réussi, false sinon
     */
    XIS_API bool Initialize(void* device, void* deviceContext, const XISConfig& config);
    
    /**
     * @brief Traite une frame avec DirectX 11
     * 
     * @param sourceTexture Texture source (ID3D11Texture2D*)
     * @param outputTexture Texture de sortie (ID3D11Texture2D*)
     * @param deviceContext Contexte de périphérique (ID3D11DeviceContext*)
     * @param params Paramètres supplémentaires (peut être nullptr pour utiliser les paramètres par défaut)
     * @return true si le traitement a réussi, false sinon
     */
    XIS_API bool ProcessFrame(void* sourceTexture, void* outputTexture, void* deviceContext, const XISParameters* params = nullptr);
}

/**
 * @brief Fonctions d'intégration spécifiques à DirectX 12
 */
namespace DX12 {
    /**
     * @brief Initialise XIS avec un périphérique DirectX 12
     * 
     * @param device Pointeur vers le périphérique ID3D12Device
     * @param commandQueue Pointeur vers la file d'attente de commandes ID3D12CommandQueue
     * @param config Configuration du système
     * @return true si l'initialisation a réussi, false sinon
     */
    XIS_API bool Initialize(void* device, void* commandQueue, const XISConfig& config);
    
    /**
     * @brief Traite une frame avec DirectX 12
     * 
     * @param sourceTexture Ressource source (ID3D12Resource*)
     * @param outputTexture Ressource de sortie (ID3D12Resource*)
     * @param commandList Liste de commandes (ID3D12GraphicsCommandList*)
     * @param params Paramètres supplémentaires (peut être nullptr pour utiliser les paramètres par défaut)
     * @return true si le traitement a réussi, false sinon
     */
    XIS_API bool ProcessFrame(void* sourceTexture, void* outputTexture, void* commandList, const XISParameters* params = nullptr);
}

} // namespace XIS