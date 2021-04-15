import React from 'react'
import { withRouter } from 'react-router-dom'
import { Route } from 'react-router'
import { render, unmountComponentAtNode } from 'react-dom'
import { ConnectedRouter as Router } from 'react-router-redux'
import { AppContainer } from 'react-hot-loader'
import { Provider } from 'react-redux'
import { configureStore, history } from './redux/store'
import { CookiesProvider, withCookies } from 'react-cookie'
import App from './App'
import 'normalize.css'
import './themes/_base.scss'

const initialState = {
  user: {
    isAuthenticated: false
  },
  vehicles: {
    currentVehicle: null,
    vehicles: []
  },
  notifications: { notifications: [] },
  commands: []
}
const store = configureStore(initialState)
const mountNode = document.getElementById('root')

const renderApp = () => {
  const AppWithRouter = withCookies(withRouter(App))
  render(
    <AppContainer>
      <CookiesProvider>
        <Provider store={store}>
          <Router history={history}>
            <Route component={AppWithRouter} />
          </Router>
        </Provider>
      </CookiesProvider>
    </AppContainer>,
    mountNode
  )
}

if (module.hot) {
  const reRenderApp = () => {
    try {
      renderApp()
    } catch (error) {
      const RedBox = require('redbox-react').default

      render(<RedBox error={error} />, mountNode)
    }
  }

  module.hot.accept('./App', () => {
    setImmediate(() => {
      // Prevents the hot reloading error from react-router
      unmountComponentAtNode(mountNode)
      reRenderApp()
    })
  })
}

renderApp()
