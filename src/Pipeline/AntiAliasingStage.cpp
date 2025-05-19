#include "AntiAliasingStage.h"
#include "../Renderer/IRenderer.h"
#include "../Core/XISContext.h"
#include "../Utils/Logger.h"

namespace XIS {

AntiAliasingStage::AntiAliasingStage(std::shared_ptr<IRenderer> renderer)
    : m_renderer(renderer),
      m_quality(AAQuality::Medium),
      m_aaShader(nullptr),
      m_aaConstantBuffer(nullptr)
{
    // Initialiser les paramètres par défaut
    m_aaParams.threshold = 0.1f;
    m_aaParams.blendFactor = 0.5f;
    m_aaParams.kernelSize = 3;
    m_aaParams.reserved = 0.0f;
}

AntiAliasingStage::~AntiAliasingStage()
{
    // Libérer les ressources
    if (m_aaShader) {
        m_renderer->ReleaseShaderResource(m_aaShader);
        m_aaShader = nullptr;
    }
    
    if (m_aaConstantBuffer) {
        m_renderer->ReleaseBuffer(m_aaConstantBuffer);
        m_aaConstantBuffer = nullptr;
    }
}

bool AntiAliasingStage::Initialize(AAQuality quality)
{
    m_quality = quality;
    
    // Si l'antialiasing est désactivé, pas besoin de créer des ressources
    if (quality == AAQuality::Off) {
        return true;
    }
    
    // Configurer les paramètres en fonction de la qualité
    switch (quality) {
        case AAQuality::Low:
            m_aaParams.threshold = 0.15f;
            m_aaParams.blendFactor = 0.3f;
            m_aaParams.kernelSize = 1;
            break;
            
        case AAQuality::Medium:
            m_aaParams.threshold = 0.1f;
            m_aaParams.blendFactor = 0.5f;
            m_aaParams.kernelSize = 3;
            break;
            
        case AAQuality::High:
            m_aaParams.threshold = 0.05f;
            m_aaParams.blendFactor = 0.7f;
            m_aaParams.kernelSize = 5;
            break;
            
        default:
            break;
    }
    
    // Créer les ressources des shaders
    return CreateShaderResources();
}

bool AntiAliasingStage::CreateShaderResources()
{
    // Charger le shader d'antialiasing depuis le fichier
    m_aaShader = m_renderer->LoadShader("AntiAliasing.hlsl", "PSAntiAliasing");
    if (!m_aaShader) {
        Logger::Error("Échec du chargement du shader d'antialiasing");
        return false;
    }
    
    // Créer le tampon constant pour les paramètres
    m_aaConstantBuffer = m_renderer->CreateConstantBuffer(sizeof(AAParams));
    if (!m_aaConstantBuffer) {
        Logger::Error("Échec de la création du tampon constant pour l'antialiasing");
        return false;
    }
    
    // Mettre à jour le tampon constant avec les paramètres initiaux
    UpdateConstantBuffer();
    
    return true;
}

void AntiAliasingStage::UpdateConstantBuffer()
{
    if (m_aaConstantBuffer) {
        m_renderer->UpdateBuffer(m_aaConstantBuffer, &m_aaParams, sizeof(AAParams));
    }
}

bool AntiAliasingStage::Process(void* inputTexture, void* outputTexture)
{
    // Si l'antialiasing est désactivé, copier simplement l'entrée vers la sortie
    if (m_quality == AAQuality::Off) {
        m_renderer->CopyResource(inputTexture, outputTexture);
        return true;
    }
    
    // Appliquer l'antialiasing en fonction de la qualité
    switch (m_quality) {
        case AAQuality::Low:
            return ApplyLowQualityAA(inputTexture, outputTexture);
            
        case AAQuality::Medium:
            return ApplyMediumQualityAA(inputTexture, outputTexture);
            
        case AAQuality::High:
            return ApplyHighQualityAA(inputTexture, outputTexture);
            
        default:
            // Ne devrait jamais arriver si nous avons vérifié correctement
            m_renderer->CopyResource(inputTexture, outputTexture);
            return true;
    }
}

void AntiAliasingStage::SetQuality(AAQuality quality)
{
    if (m_quality != quality) {
        m_quality = quality;
        
        // Mettre à jour les paramètres en fonction de la nouvelle qualité
        switch (quality) {
            case AAQuality::Low:
                m_aaParams.threshold = 0.15f;
                m_aaParams.blendFactor = 0.3f;
                m_aaParams.kernelSize = 1;
                break;
                
            case AAQuality::Medium:
                m_aaParams.threshold = 0.1f;
                m_aaParams.blendFactor = 0.5f;
                m_aaParams.kernelSize = 3;
                break;
                
            case AAQuality::High:
                m_aaParams.threshold = 0.05f;
                m_aaParams.blendFactor = 0.7f;
                m_aaParams.kernelSize = 5;
                break;
                
            default:
                break;
        }
        
        // Mettre à jour le tampon constant
        UpdateConstantBuffer();
    }
}

bool AntiAliasingStage::ApplyLowQualityAA(void* input, void* output)
{
    // Configuration pour qualité faible (FXAA simplifié)
    m_renderer->SetShader(m_aaShader);
    m_renderer->SetConstantBuffer(m_aaConstantBuffer, 0);
    m_renderer->SetTexture(input, 0);
    m_renderer->SetRenderTarget(output);
    
    // Exécuter le shader avec le tampon constant actuel
    bool success = m_renderer->ExecuteShader();
    if (!success) {
        Logger::Error("Échec de l'exécution du shader d'antialiasing (qualité faible)");
    }
    
    return success;
}

bool AntiAliasingStage::ApplyMediumQualityAA(void* input, void* output)
{
    // Configuration pour qualité moyenne (FXAA standard)
    m_renderer->SetShader(m_aaShader);
    m_renderer->SetConstantBuffer(m_aaConstantBuffer, 0);
    m_renderer->SetTexture(input, 0);
    m_renderer->SetRenderTarget(output);
    
    // Exécuter le shader avec le tampon constant actuel
    bool success = m_renderer->ExecuteShader();
    if (!success) {
        Logger::Error("Échec de l'exécution du shader d'antialiasing (qualité moyenne)");
    }
    
    return success;
}

bool AntiAliasingStage::ApplyHighQualityAA(void* input, void* output)
{
    // Configuration pour qualité élevée (FXAA avancé ou SMAA)
    m_renderer->SetShader(m_aaShader);
    m_renderer->SetConstantBuffer(m_aaConstantBuffer, 0);
    m_renderer->SetTexture(input, 0);
    m_renderer->SetRenderTarget(output);
    
    // Exécuter le shader avec le tampon constant actuel
    bool success = m_renderer->ExecuteShader();
    if (!success) {
        Logger::Error("Échec de l'exécution du shader d'antialiasing (qualité élevée)");
    }
    
    return success;
}

} // namespace XIS