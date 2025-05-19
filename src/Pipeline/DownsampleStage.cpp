#include "DownsampleStage.h"
#include "../Renderer/IRenderer.h"
#include "../Utils/Logger.h"

namespace XIS {

DownsampleStage::DownsampleStage(std::shared_ptr<IRenderer> renderer)
    : m_renderer(renderer),
      m_downsampleShader(nullptr),
      m_constantBuffer(nullptr)
{
    // Initialiser les paramètres par défaut
    m_params.downsampleFactor = 0.5f;  // Par défaut, réduire à 50%
    m_params.preserveDetail = 0.75f;   // Conserver 75% des détails
    m_params.threshold = 0.1f;         // Seuil pour la détection des contours
    m_params.reserved = 0.0f;
}

DownsampleStage::~DownsampleStage()
{
    // Libérer les ressources
    if (m_downsampleShader) {
        m_renderer->ReleaseShaderResource(m_downsampleShader);
        m_downsampleShader = nullptr;
    }
    
    if (m_constantBuffer) {
        m_renderer->ReleaseBuffer(m_constantBuffer);
        m_constantBuffer = nullptr;
    }
}

bool DownsampleStage::Initialize()
{
    return CreateShaderResources();
}

bool DownsampleStage::CreateShaderResources()
{
    // Charger le shader de downsampling depuis le fichier
    m_downsampleShader = m_renderer->LoadShader("Downsample.hlsl", "PSDownsample");
    if (!m_downsampleShader) {
        Logger::Error("Échec du chargement du shader de downsampling");
        return false;
    }
    
    // Créer le tampon constant pour les paramètres
    m_constantBuffer = m_renderer->CreateConstantBuffer(sizeof(DownsampleParams));
    if (!m_constantBuffer) {
        Logger::Error("Échec de la création du tampon constant pour le downsampling");
        return false;
    }
    
    // Mettre à jour le tampon constant avec les paramètres initiaux
    UpdateConstantBuffer();
    
    return true;
}

void DownsampleStage::UpdateConstantBuffer()
{
    if (m_constantBuffer) {
        m_renderer->UpdateBuffer(m_constantBuffer, &m_params, sizeof(DownsampleParams));
    }
}

bool DownsampleStage::Process(void* inputTexture, void* outputTexture, float factor)
{
    // Si un facteur spécifique est fourni, l'utiliser temporairement
    if (factor > 0.0f) {
        float oldFactor = m_params.downsampleFactor;
        m_params.downsampleFactor = factor;
        UpdateConstantBuffer();
        
        // Exécuter le shader
        bool result = Process(inputTexture, outputTexture);
        
        // Restaurer l'ancien facteur
        m_params.downsampleFactor = oldFactor;
        UpdateConstantBuffer();
        
        return result;
    }
    
    // Configuration standard pour le downsampling
    m_renderer->SetShader(m_downsampleShader);
    m_renderer->SetConstantBuffer(m_constantBuffer, 0);
    m_renderer->SetTexture(inputTexture, 0);
    m_renderer->SetRenderTarget(outputTexture);
    
    // Exécuter le shader avec le tampon constant actuel
    bool success = m_renderer->ExecuteShader();
    if (!success) {
        Logger::Error("Échec de l'exécution du shader de downsampling");
    }
    
    return success;
}

void DownsampleStage::SetDownsampleFactor(float factor)
{
    // Limiter le facteur entre 0.1 et 1.0
    factor = std::max(0.1f, std::min(1.0f, factor));
    
    if (m_params.downsampleFactor != factor) {
        m_params.downsampleFactor = factor;
        UpdateConstantBuffer();
    }
}

} // namespace XIS