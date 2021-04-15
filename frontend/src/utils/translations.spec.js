import * as translations from './translations'

describe('translations.js', () => {
  describe('scaleAtoB', () => {
    it('should correctly scale an A range value to a B range value', () => {
      const value = 5
      const aMin = 0
      const aMax = 10
      const bMin = 0
      const bMax = 100

      const result = translations.scaleAtoB(value, aMin, aMax, bMin, bMax)

      expect(result).toBe(50)
    })

    it("should correctly scale an A range value when B's range is a single value", () => {
      const value = 5
      const aMin = 0
      const aMax = 10
      const bMin = 100
      const bMax = 100

      const result = translations.scaleAtoB(value, aMin, aMax, bMin, bMax)

      expect(result).toBe(100)
    })
  })
})
