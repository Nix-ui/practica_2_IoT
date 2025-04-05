class LedController : public Controller {
    public:
        LedController(Led* leds, byte numLeds) 
            : m_leds(leds), 
              m_numLeds(numLeds) {}
        
        void setAllState(byte state) override {
            for (byte i = 0; i < m_numLeds; ++i) {
                m_leds[i].setState(state);
            }
        }
    
        void setState(byte ledPin, byte state) override {
            for (byte i = 0; i < m_numLeds; ++i) {
                if (m_leds[i].getPin() == ledPin) {
                    m_leds[i].setState(state);
                    return;
                }
            }
            // Opcional: manejo de error si el pin no existe
        }
    
    private:
        Led* m_leds;
        byte m_numLeds;
    };