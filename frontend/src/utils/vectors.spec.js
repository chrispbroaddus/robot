import * as vectors from './vectors'

describe('vectors.js', () => {
  describe('Vector2', () => {
    it('should create a new 2-vector with the correct (x,y) coordinates (integers)', () => {
      const x = -1
      const y = 1
      const result = new vectors.Vector2(x, y)

      expect(result.x).toEqual(x)
      expect(result.y).toEqual(y)
    })
    it('should create a new 2-vector with the correct (x,y) coordinates (floats)', () => {
      const x = -1.2295414135
      const y = 2.38093149814958
      const result = new vectors.Vector2(x, y)

      expect(result.x).toBeCloseTo(x)
      expect(result.y).toBeCloseTo(y)
    })
  })

  describe('Vector3', () => {
    it('should create a new 3-vector with the correct (x,y,z) coordinates (integers)', () => {
      const x = -1
      const y = 1
      const z = -6
      const result = new vectors.Vector3(x, y, z)

      expect(result.x).toEqual(x)
      expect(result.y).toEqual(y)
      expect(result.z).toEqual(z)
    })
    it('should create a new 3-vector with the correct (x,y,z) coordinates (floats)', () => {
      const x = -1.2295414135
      const y = 2.38093149814958
      const z = 1.124814012398
      const result = new vectors.Vector3(x, y, z)

      expect(result.x).toBeCloseTo(x)
      expect(result.y).toBeCloseTo(y)
      expect(result.z).toBeCloseTo(z)
    })
  })
})
