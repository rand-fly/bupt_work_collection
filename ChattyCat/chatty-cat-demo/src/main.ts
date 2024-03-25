import { createApp } from 'vue';
import ElementPlus from 'element-plus';
import { install as VueMonacoEditorPlugin } from '@guolao/vue-monaco-editor';
import './assets/main.css';
import 'element-plus/dist/index.css';
import App from './App.vue';

const app = createApp(App);

app.use(ElementPlus);
app.use(VueMonacoEditorPlugin, {
  paths: {
    // The recommended CDN config
    vs: 'https://cdn.jsdelivr.net/npm/monaco-editor@0.43.0/min/vs'
  }
});

app.mount('#app');
