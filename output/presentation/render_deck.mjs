import fs from 'node:fs/promises';
import {PresentationFile,FileBlob} from 'file:///C:/Users/paradise/.cache/codex-runtimes/codex-primary-runtime/dependencies/node/node_modules/@oai/artifact-tool/dist/artifact_tool.mjs';
const p=await PresentationFile.importPptx(await FileBlob.load('output/presentation/Monster_Clash_Defence_Final.pptx'));
for(let i=0;i<p.slides.items.length;i++){
    const b=await p.export({slide:p.slides.items[i],format:'png',scale:2});
    await fs.writeFile(`tmp/presentation/local/final-${String(i+1).padStart(2,'0')}.png`,new Uint8Array(await b.arrayBuffer()));
}
console.log('Rendered exact finalized PowerPoint at 2x');
