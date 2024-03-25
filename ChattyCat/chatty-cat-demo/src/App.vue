<script setup lang="ts">
import { computed, onMounted, ref, shallowRef, triggerRef, watch, watchEffect } from 'vue';
import ChatBox from './ChatBox.vue';
import { Edit, Delete, Plus, ArrowDown } from '@element-plus/icons-vue';
import { parse, Chat, type Script } from 'chatty-cat';
import { ElMessage } from 'element-plus';
import { useMonaco } from '@guolao/vue-monaco-editor';

import { monarch } from './monarch';
import { examples } from './examples';

const monacoEditorOptions = {
  automaticLayout: true,
  wordWrap: 'on'
};

const { monacoRef } = useMonaco();
watchEffect(() => {
  const monaco = monacoRef.value;
  if (monaco) {
    monaco.languages.register({ id: 'ccsl' });
    monaco.languages.setMonarchTokensProvider('ccsl', monarch as any);
  }
});

const scriptCode = ref(examples[0].ccsl);
const funcCode = ref(examples[0].js);
const error = ref('');
const applied = ref(true);
const chat = shallowRef(compile()!);
const chatBox = ref<InstanceType<typeof ChatBox> | null>(null);

onMounted(() => {
  chat.value.start();
});

function send(content: string) {
  chat.value.input(content);
}

function apply() {
  const newChat = compile();
  if (newChat) {
    chat.value.stop();
    chatBox.value?.clear();
    applied.value = true;
    error.value = '';
    chat.value = newChat;
    chat.value.start();
  }
}

function compile() {
  let script: Script;
  let funcs: any;
  try {
    script = parse(scriptCode.value);
  } catch (e) {
    error.value = '编译错误 ' + (e as Error).message;
    return;
  }
  try {
    funcs = new Function(funcCode.value)();
  } catch (e) {
    error.value = '外部函数错误 ' + (e as Error).message;
    return;
  }
  return new Chat({
    script,
    output(text) {
      chatBox.value?.receive(text);
    },
    suggest(texts) {
      chatBox.value?.suggest(texts);
    },
    error(e) {
      error.value = '运行时错误 ' + e;
      chatBox.value?.botError('机器人发生了内部错误');
    },
    update() {
      triggerRef(chat);
    },
    funcs
  });
}

watch([scriptCode, funcCode], () => {
  applied.value = false;
});

const variableTable = computed(() => {
  const entries = [];
  for (const [name, value] of Object.entries(chat.value.variables)) {
    entries.push({ name, value });
  }
  return entries;
});

const varDialog = ref({
  visible: false,
  add: false,
  name: '',
  value: ''
});

function editVar(name: string, value: string) {
  varDialog.value = {
    visible: true,
    add: false,
    name,
    value
  };
}

function deleteVar(name: string) {
  delete chat.value.variables[name];
  triggerRef(chat);
}

function addVar() {
  varDialog.value = {
    visible: true,
    add: true,
    name: '$',
    value: ''
  };
}

function varDialogConfirm() {
  if (!varDialog.value.name.match(/^\$[_a-zA-Z]+$/)) {
    ElMessage({
      message: '变量名无效',
      type: 'error'
    });
    return;
  }
  if (varDialog.value.add && chat.value.variables[varDialog.value.name] !== undefined) {
    ElMessage({
      message: `变量 ${varDialog.value.name} 已存在`,
      type: 'error'
    });
    return;
  }
  chat.value.variables[varDialog.value.name] = varDialog.value.value;
  triggerRef(chat);
  varDialog.value.visible = false;
}
</script>

<template>
  <div class="container">
    <div class="col dev-panel">
      <el-tabs type="border-card" class="editor">
        <el-tab-pane label="脚本(ccsl)" class="editor-pane">
          <vue-monaco-editor
            v-model:value="scriptCode"
            :options="monacoEditorOptions"
            language="ccsl"
          />
        </el-tab-pane>
        <el-tab-pane label="外部函数(javascript)" class="editor-pane">
          <vue-monaco-editor
            v-model:value="funcCode"
            :options="monacoEditorOptions"
            language="javascript"
          />
        </el-tab-pane>
      </el-tabs>
      <el-tabs type="border-card">
        <el-tab-pane class="tab-pane" label="状态">
          <div style="display: flex; flex-direction: row">
            <el-result
              :icon="applied ? 'success' : 'warning'"
              :title="applied ? '已应用' : '未应用'"
            >
            </el-result>
            <div>
              <br />
              <el-button type="primary" @click="apply">应用脚本</el-button>
              <el-dropdown>
                <el-button style="margin-left: 10px">
                  加载示例脚本
                  <el-icon class="el-icon--right">
                    <ArrowDown />
                  </el-icon>
                </el-button>
                <template #dropdown>
                  <el-dropdown-menu>
                    <el-dropdown-item
                      v-for="i in examples"
                      @click="
                        scriptCode = i.ccsl;
                        funcCode = i.js;
                      "
                      :key="i.name"
                    >
                      {{ i.name }}
                    </el-dropdown-item>
                  </el-dropdown-menu>
                </template>
              </el-dropdown>
              <br /><br />
              当前状态：{{ chat.exited ? '已结束' : chat.currentState.name }}
              {{ chat.suspending ? '(挂起中)' : '' }} <br /><br />
              {{ error }}
            </div>
          </div>
        </el-tab-pane>
        <el-tab-pane class="tab-pane" label="变量">
          <div>
            <el-table :data="variableTable" height="155" empty-text="无变量">
              <el-table-column prop="name" label="变量名" width="100" />
              <el-table-column prop="value" label="值" />
              <el-table-column label="操作" fixed="right" width="100">
                <template #default="scope">
                  <el-button
                    link
                    :icon="Edit"
                    @click="editVar(scope.row.name, scope.row.value)"
                  ></el-button>
                  <el-button
                    link
                    :icon="Delete"
                    type="danger"
                    @click="deleteVar(scope.row.name)"
                  ></el-button>
                </template>
              </el-table-column>
            </el-table>
            <el-button
              :icon="Plus"
              type="primary"
              text
              bg
              class="add-button"
              @click="addVar"
            ></el-button>
          </div>
        </el-tab-pane>
      </el-tabs>
    </div>
    <div class="chat-box">
      <ChatBox ref="chatBox" @send="send" :exited="chat.exited" :suspending="chat.suspending" />
    </div>
  </div>

  <el-dialog
    v-model="varDialog.visible"
    width="500"
    :title="varDialog.add ? '添加变量' : '修改变量'"
  >
    <el-form label-position="right" label-width="60px">
      <el-form-item label="变量名">
        <el-input
          v-model="varDialog.name"
          :disabled="!varDialog.add"
          @input="
            varDialog.name = varDialog.name[0] === '$' ? varDialog.name : '$' + varDialog.name
          "
        />
      </el-form-item>
      <el-form-item label="值">
        <el-input v-model="varDialog.value" />
      </el-form-item>
    </el-form>
    <template #footer>
      <span class="dialog-footer">
        <el-button @click="varDialog.visible = false">取消</el-button>
        <el-button type="primary" @click="varDialogConfirm()">确认</el-button>
      </span>
    </template>
  </el-dialog>
</template>

<style scoped>
.container {
  display: flex;
  flex-direction: row;
  width: 100vw;
  height: 100vh;
}

.dev-panel {
  width: 56%;
  display: flex;
  flex-direction: column;
  border-right: 1px solid #ccc;
}

.editor {
  flex-grow: 1;
  display: flex;
  flex-direction: column;
}

.editor-pane {
  height: 100%;
}

.chat-box {
  flex-grow: 1;
}

.tab-pane {
  height: 200px;
  overflow: auto;
}

.add-button {
  margin-top: 10px;
  width: 100%;
}
</style>

<style>
.editor > .el-tabs__content {
  flex-grow: 1;
  padding: 0;
}
</style>
