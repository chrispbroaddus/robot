export const createReducer = (initialState, handlers) =>
  function reducer(state = initialState, action = {}) {
    if (Object.prototype.hasOwnProperty.call(handlers, action.type)) {
      return handlers[action.type](state, action)
    }
    return state
  }

/**
 * Patch empty fields in an object with a specified value. Mutates the object.
 * @param  {Object} object       Object to patch
 * @param  {Array} fields        Array of keys in the object to patch
 * @param  {Number} defaultValue Value for the patch (optional)
 */
export const patchEmptyFields = (object, fields, defaultValue = 0) => {
  if (!object) {
    return new Error("Target object can't be null")
  }

  for (let k = 0; k < fields.length; k++) {
    if (object[fields[k]] === undefined) {
      object[fields[k]] = defaultValue
    }
  }
}
