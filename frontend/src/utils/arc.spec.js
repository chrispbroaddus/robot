import { manifestFactory } from './manifest'
import * as arc from './arc'
import { Vector2 } from './vectors'

describe('arc.js', () => {
  describe('drawArcs', () => {
    let x
    let y
    let mousePoint

    let ctx
    let manifest = manifestFactory()
    let intrinsics
    let canvasDimensions

    beforeEach(() => {
      // Using a random (x, y) for now
      x = 0
      y = 0

      mousePoint = new Vector2(x, y)

      ctx = {
        stroke: jest.fn(),
        setLineDash: jest.fn()
      }
      intrinsics = manifest.intrinsics
      canvasDimensions = {
        x: 2000,
        y: 2000
      }
    })

    xit(
      'should draw an arc by calling the stroke on the canvas (basic)',
      () => {
        mousePoint = new Vector2(-1, 1)

        arc.drawArcs(
          ctx,
          mousePoint,
          intrinsics,
          canvasDimensions,
          arc.CAMERA_HEIGHT,
          arc.ARC_SAMPLE_SIZE,
          arc.VEHICLE_WIDTH
        )

        expect(ctx.stroke).toHaveBeenCalled()
      }
    )
  })
})
