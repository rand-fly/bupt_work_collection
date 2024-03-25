import { config, mount } from '@vue/test-utils';
import { test, expect } from 'vitest';
import ChatBox from '../ChatBox.vue';
import ElementPlus from 'element-plus';
import { nextTick } from 'vue';

config.global.plugins = [ElementPlus];

test('adds user message', async () => {
  const wrapper = mount(ChatBox, {
    props: { exited: false, suspending: true }
  });
  const input = wrapper.find('textarea');
  const sendButton = wrapper.find('button[type="button"]');

  await input.setValue('a');
  await sendButton.trigger('click');
  await input.setValue('b');
  await input.trigger('keydown.enter');
  await wrapper.setProps({ suspending: false });
  await input.setValue('c');
  await sendButton.trigger('click');
  await input.setValue('d');
  await input.trigger('keydown.enter');

  expect(wrapper.findAll('.user-msg')[0].text()).toContain('c');
  expect(wrapper.findAll('.user-msg')[1].text()).toContain('d');
});

test('adds bot message when receive is called', async () => {
  const wrapper = mount(ChatBox, {
    props: { exited: false, suspending: false }
  });
  wrapper.vm.receive('Hello from bot');
  await nextTick();

  expect(wrapper.find('.bot-msg').text()).toContain('Hello from bot');
});

test('adds user message when suggestion button is clicked', async () => {
  const wrapper = mount(ChatBox, {
    props: { exited: false, suspending: false }
  });
  wrapper.vm.suggest(['Suggestion Content 1', 'Suggestion Content 2']);
  await nextTick();

  const suggestionButton = wrapper.findAll('.suggestion')[1];
  await suggestionButton.trigger('click');
  expect(wrapper.find('.user-msg').text()).toContain('Suggestion Content 2');
  expect(wrapper.findAll('.suggestion').length).toBe(0);
});

test('adds bot error message when botError method is called', async () => {
  const wrapper = mount(ChatBox, {
    props: { exited: false, suspending: false }
  });
  expect(wrapper.find('.bot-error').exists()).toBe(false);
  wrapper.vm.botError('Error occurred');
  await nextTick();
  expect(wrapper.find('.bot-error').exists()).toBe(true);
});

test('clear messages', async () => {
  const wrapper = mount(ChatBox, {
    props: { exited: false, suspending: false }
  });
  wrapper.vm.receive('Hello from bot');
  wrapper.vm.botError('Error occurred');
  wrapper.vm.suggest(['Suggestion Content 1', 'Suggestion Content 2']);
  await nextTick();
  const input = wrapper.find('textarea');
  await input.setValue('hello');
  await input.trigger('keydown.enter');
  wrapper.vm.clear();
  await nextTick();
  expect(wrapper.find('.user').exists()).toBe(false);
  expect(wrapper.find('.bot').exists()).toBe(false);
});
