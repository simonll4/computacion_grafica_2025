#pragma once
#include "Instrument.h"

namespace hud
{
    /**
     * @class WaypointIndicator
     * @brief Indicador de navegación por waypoints
     *
     * Este instrumento proporciona ayuda visual para navegación mediante
     * waypoints, mostrando:
     * - Dirección al siguiente waypoint (flecha direccional)
     * - Distancia al waypoint objetivo
     * - Ángulo de desviación respecto al rumbo actual
     *
     * Ayuda al piloto a mantener la aeronave en ruta sin consultar
     * constantemente mapas o cartas de navegación.
     */
    class WaypointIndicator : public Instrument
    {
    public:
        WaypointIndicator();

        /**
         * @brief Renderiza el indicador de waypoint con los datos actuales
         * @param renderer Renderer 2D compartido
         * @param flightData Datos del vuelo (especialmente waypoint info)
         */
        void render(gfx::Renderer2D &renderer, const flight::FlightData &flightData) override;

    private:
        // Métodos específicos del indicador de waypoint
        void drawBackground(gfx::Renderer2D &renderer);
        void drawDirectionArrow(gfx::Renderer2D &renderer, float bearing, float heading);
        void drawDistanceInfo(gfx::Renderer2D &renderer, float distance);
        void drawNoWaypointMessage(gfx::Renderer2D &renderer);
    };

} // namespace hud
