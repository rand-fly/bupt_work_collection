<script setup lang="ts">
import { nextTick, ref } from 'vue';
import { Promotion } from '@element-plus/icons-vue';

const emit = defineEmits(['send']);
defineExpose({ receive, botError, suggest, clear });
const props = defineProps<{ exited: boolean; suspending: boolean }>();

const msg = ref([] as { sender: 'user' | 'bot' | 'botError'; content: string; key: number }[]);
const suggestions = ref([] as string[]);

const input = ref('');
const msgList = ref<HTMLElement | null>(null);

function send() {
  if (props.exited || props.suspending || input.value.trim() === '') return;
  addMsg('user', input.value);
  emit('send', input.value);
  input.value = '';
}

function sendSuggestion(content: string) {
  suggestions.value = [];
  addMsg('user', content);
  emit('send', content);
}

function receive(content: string) {
  addMsg('bot', content);
}

function botError(error: string) {
  addMsg('botError', error);
}

function suggest(content: string[]) {
  suggestions.value = content;
  nextTick(() => {
    msgList.value?.scrollTo(0, msgList.value.scrollHeight);
  });
}

function clear() {
  msg.value = [];
  suggestions.value = [];
}

function addMsg(sender: 'user' | 'bot' | 'botError', content: string) {
  msg.value.push({ sender, content, key: msg.value.length });
  nextTick(() => {
    msgList.value?.scrollTo(0, msgList.value.scrollHeight);
  });
}
</script>

<template>
  <div class="box">
    <div class="msg-list" ref="msgList">
      <div v-for="i in msg" :key="i.key">
        <div v-if="i.sender === 'user'" class="user">
          <div class="user-msg">{{ i.content }}</div>
        </div>
        <div v-else-if="i.sender === 'bot'" class="bot">
          <div class="bot-msg">{{ i.content }}</div>
        </div>
        <div v-else>
          <el-divider class="bot-error">{{ i.content }}</el-divider>
        </div>
      </div>
      <div v-if="suspending" class="bot">
        <div v-loading="suspending" style="height: 50px; width: 50px"></div>
      </div>
      <el-divider v-if="exited">对话已结束</el-divider>
    </div>
    <div class="suggestion-section">
      <el-button
        v-for="i in suggestions"
        :key="i"
        type="primary"
        class="suggestion"
        round
        text
        bg
        @click="sendSuggestion(i)"
      >
        {{ i }}
      </el-button>
    </div>
    <div class="input">
      <el-input
        v-model="input"
        type="textarea"
        resize="none"
        autosize
        clearable
        @keydown.enter.prevent="send"
      />
      <el-button
        type="primary"
        :icon="Promotion"
        style="margin-left: 10px; height: 100%"
        @click="send"
        :disabled="exited || suspending"
        >发送</el-button
      >
    </div>
  </div>
</template>

<style scoped>
.box {
  height: 100%;
  width: 100%;
  display: flex;
  flex-direction: column;
}

.msg-list {
  flex-grow: 1;
  overflow-y: scroll;
}

.user {
  display: flex;
  flex-direction: row-reverse;
  margin: 15px;
  padding-left: 30px;
}

.user-msg {
  background-color: #007aff;
  border-radius: 10px;
  padding: 10px;
  color: white;
  word-break: break-word;
}

.bot {
  display: flex;
  flex-direction: row;
  margin: 15px;
  padding-right: 30px;
}

.bot-msg {
  background-color: #e5e5ea;
  border-radius: 10px;
  padding: 10px;
  word-break: break-word;
}

.input {
  display: flex;
  flex-direction: row;
  margin: 10px;
}

.suggestion-section {
  display: flex;
  flex-flow: row wrap;
  justify-content: center;
}

.suggestion {
  margin: 5px;
}
</style>
