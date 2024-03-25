import { config, flushPromises, mount } from '@vue/test-utils';
import { test, expect, vi } from 'vitest';
import ElementPlus from 'element-plus';
import App from '../App.vue';

vi.mock('@guolao/vue-monaco-editor', () => {
  return {
    useMonaco() {
      return { monacoRef: {} };
    }
  };
});

config.global.plugins = [ElementPlus];
config.global.stubs = {
  'vue-monaco-editor': {
    template: `<textarea :value="value" @input="$emit('update:value', $event.target.value)"></textarea>`,
    props: ['value'],
    emits: ['update:value']
  }
};

test('send function should call chat.input with the provided content', async () => {
  const wrapper = mount(App);
  await flushPromises();
  expect(wrapper.find('.bot-msg').text()).toEqual('您好，请问您有什么需要的吗？');
  await wrapper.findAll('textarea')[0].setValue(`state main enter say "hello"`);
  await wrapper.findAll('button')[0].trigger('click');
  expect(wrapper.find('.bot-msg').text()).toEqual('hello');
});
