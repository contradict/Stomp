import 'systemjs-webpack-interop/auto-public-path/2'
import Vue from 'vue'
import singleSpaVue from 'single-spa-vue'

import App from './App.vue'
import router from './router'
import store from '@cosmosc2/tool-common/src/plugins/store'

// Register these globally so they don't have to be imported every time
import AstroBadge from '@cosmosc2/tool-common/src/components/icons/AstroBadge'
import AstroBadgeIcon from '@cosmosc2/tool-common/src/components/icons/AstroBadgeIcon'
Vue.component('astro-badge', AstroBadge)
Vue.component('astro-badge-icon', AstroBadgeIcon)

Vue.config.productionTip = false

import '@cosmosc2/tool-common/src/assets/stylesheets/layout/layout.scss'
import vuetify from './plugins/vuetify'
import Dialog from '@cosmosc2/tool-common/src/plugins/dialog'
import PortalVue from 'portal-vue'
import Notify from '@cosmosc2/tool-common/src/plugins/notify'

Vue.use(PortalVue)
Vue.use(Dialog)
Vue.use(Notify, { store })

const vueLifecycles = singleSpaVue({
  Vue,
  appOptions: {
    router,
    store,
    vuetify,
    render(h) {
      return h(App, {
        props: {},
      })
    },
    el: '#cosmos-tool',
  },
})

export const bootstrap = vueLifecycles.bootstrap
export const mount = vueLifecycles.mount
export const unmount = vueLifecycles.unmount
