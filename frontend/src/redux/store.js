import { applyMiddleware, createStore, compose } from 'redux'
import { routerMiddleware } from 'react-router-redux'
import {
  // persistStore,
  autoRehydrate
} from 'redux-persist'
import createHistory from 'history/createBrowserHistory'
import thunk from 'redux-thunk'
import axios from 'axios'
import rootReducer from './index'

export const history = createHistory()

const middleware = [thunk.withExtraArgument(axios), routerMiddleware(history)]
if (process.env.NODE_ENV === 'development') {
  // Add development-specific middleware here
  // middleware.push(...)
}

export const configureStore = (initialState = {}) => {
  const composeEnhancers =
    window.__REDUX_DEVTOOLS_EXTENSION_COMPOSE__ || compose

  const enhancers = composeEnhancers(
    applyMiddleware(...middleware),
    autoRehydrate()
  )

  const store = createStore(rootReducer, initialState, enhancers)

  // // Loads state from local storage
  // persistStore(store, {
  //   blacklist: ['routing', 'form']
  // })

  // if (module.hot) {
  //   // Enable Webpack hot module replacement for reducers
  //   module.hot.accept('./redux', () => {
  //     const nextRootReducer = require('./redux/index').rootReducer // eslint-disable-line
  //     store.replaceReducer(nextRootReducer)
  //   })
  // }

  return store
}
