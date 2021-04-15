import * as manifest from './manifest'

describe('manifest.js', () => {
  describe('manifestFactory', () => {
    it('should return a default manifest if no manifest is provided', () => {
      const result = manifest.manifestFactory()

      expect(result.intrinsics).toBeTruthy()
      expect(result.extrinsics).toBeTruthy()
      expect(result.isDefault).toEqual(true)
    })

    it('should return a default manifest if an invalid manifest is provided', () => {
      const invalidManifest = {
        intrinsics: null,
        extrinsics: null
      }
      const result = manifest.manifestFactory(invalidManifest)

      expect(result.intrinsics).toBeTruthy()
      expect(result.extrinsics).toBeTruthy()
      expect(result.isDefault).toEqual(true)
    })
  })
})
