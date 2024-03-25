export const examples = [
  {
    name: '快递客服',
    ccsl: `state main
    enter
        say "您好，请问您有什么需要的吗？"
        goto menu
        
state menu
    enter
        suggest "我的快递怎么还没有到", "查询运单", "什么是疑难件"
    case "你好"
        say random("你好！", "你好，祝你生活愉快！")
    case /没到|没有到/
        say "很抱歉您的快递还没有到达，你可以说“查询运单”来让我帮你查询"
        suggest "查询运单"
    case /为什么/
        say "对不起，我还不能解答你的问题"
    case /查询/
        goto query
    case "什么是疑难件"
        say "疑难件是指快递在运输过程中出现问题的快件，例如：地址不详、无人签收、货物破损等。"
    default
        say random("对不起，我不能理解", "抱歉，我还不能完成这项功能")
        goto menu
    silent 5
        suggest "我的快递怎么还没有到", "查询运单", "什么是疑难件"
    silent 10
        say "您好，请问您还在吗？"

state query
    enter
        say "您是要查询运单吗？请输入您的运单号"
    case /^\\d{10}$/
        let $valid = validateNumber()        
        [$valid] say "这边帮您查询到您运单的信息是" + queryNumber()
        [not($valid)] say "抱歉，没有查询到您的运单信息"
        goto menu
    case /退出|离开|结束|不/
        goto menu
    default
        say "运单号格式不正确，请问您要退出查询吗？"
        suggest "退出"
`,
    js: `function validateNumber(number) {
    return "1"
}

function queryNumber(number) {
    return "快递即将到达"
}

return { validateNumber, queryNumber }
`
  },
  {
    name: '查询余额与充值',
    ccsl: `state main
        enter
            let $balance = "0"
            say "你好，请问你需要做什么？"
            goto menu
    
    state menu
        enter
            suggest "查询余额", "充值"
        case /余额/
            say "您的余额为" + $balance + "元"
            goto menu
        case /充值/
            goto bill
            say random("抱歉，我不能理解你的话", "您可以再说一遍吗", "我能力还有限，不能理解你的意思")
            goto menu
    
    state bill
        enter
            say "请输入充值金额"
        case /^[0-9]+$/
            let $ok = neq(numcmp($0, "1000"), "1")
            [$ok] say "充值成功"
            [$ok] let $balance = add($balance, $0)
            [$ok] goto menu
            [not($ok)] say "充值金额不能超过1000元"
        default
            say "请输入正整数"
`,
    js: ''
  },
  {
    name: '猜数字',
    ccsl: `state main
    enter
        let $ans = randomInt("1", "1000")
        say "我想了一个1到1000之间的整数，你来猜猜是多少"
    case /^[0-9]+$/
        let $cmp = numcmp($0, $ans)
        [eq($cmp, "1")] say "大了"
        [eq($cmp, "-1")] say "小了"
        [eq($cmp, "0")] say "恭喜你猜对了"
        [eq($cmp, "0")] goto main
    default
        say "请猜一个正整数"
`,
    js: ''
  },
  {
    name: '接入ChatGPT',
    ccsl: `state main
    enter
        say "请先在“外部函数”中设置你的OpenAI Key，然后输入“接入”将后续对话转发到ChatGPT"
        suggest "接入"
    case "接入"
        goto gptMode

state gptMode
    enter
        say "接下来的对话将转发到ChatGPT处理，输入“退出”结束转发"
        let $_ = initGPT()
    case "退出"
        say "结束ChatGPT转发模式"
        goto main
    case /.*/
        say queryGPT($0)
`,
    js: `const key = "OpenAI Key"
let messages = []

function initGPT() {
    messages = []
    return ""
}

async function queryGPT(input) {
    messages.push({ "role": "user", "content": input })
    const res = await fetch("https://api.openai.com/v1/chat/completions", {
        method: "POST",
        headers: {
            "Content-Type": "application/json",
            "Authorization": "Bearer " + key
        },
        body: JSON.stringify({
            "model": "gpt-3.5-turbo",
            "messages": messages,
            "temperature": 0.7
        })
    })
    const msg = (await res.json()).choices[0].message.content
    messages.push({ "role": "assistant", "content": msg })
    return msg
}
        
// return 的函数才能被脚本调用
return { initGPT, queryGPT }
`
  }
];
