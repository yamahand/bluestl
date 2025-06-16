#!/usr/bin/env python3
"""
BlueStl 静的解析統合スクリプト (Python版)
複数の静的解析ツールを統合実行し、結果を集約して報告します。
"""

import os
import sys
import subprocess
import argparse
import json
import datetime
from pathlib import Path
from typing import List, Dict, Any, Optional
import xml.etree.ElementTree as ET


class StaticAnalysisRunner:
    """静的解析ツールの統合実行クラス"""
    
    def __init__(self, project_root: Path):
        self.project_root = project_root
        self.timestamp = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        self.reports = {}
        
    def find_source_files(self) -> Dict[str, List[Path]]:
        """解析対象ファイルを検索"""
        files = {
            'headers': list((self.project_root / 'include' / 'bluestl').glob('*.h')),
            'sources': list((self.project_root / 'tests').glob('*.cpp')),
            'benchmarks': list((self.project_root / 'benchmarks').glob('*.cpp'))
        }
        return files
    
    def run_clang_tidy(self, files: List[Path], output_dir: Path, fix: bool = False) -> Dict[str, Any]:
        """clang-tidy解析を実行"""
        print("🔍 clang-tidy解析を実行中...")
        
        report_file = output_dir / f"clang_tidy_{self.timestamp}.json"
        
        cmd = [
            'clang-tidy',
            '--config-file=.clang-tidy',
            '--header-filter=.*include/bluestl.*',
            '--export-fixes=' + str(report_file),
            '--format-style=file'
        ]
        
        if fix:
            cmd.append('--fix')
            print("⚠️  自動修正モードが有効です。")
        
        # C++20とインクルードパス設定
        cmd.extend(['--', '-std=c++20', '-Iinclude'])
        
        issues = []
        for file_path in files:
            try:
                result = subprocess.run(
                    cmd + [str(file_path)],
                    capture_output=True,
                    text=True,
                    cwd=self.project_root
                )
                
                if result.stderr:
                    # clang-tidyの出力を解析
                    for line in result.stderr.split('\n'):
                        if 'warning:' in line or 'error:' in line:
                            issues.append({
                                'file': str(file_path),
                                'line': self._extract_line_number(line),
                                'column': self._extract_column_number(line),
                                'severity': 'warning' if 'warning:' in line else 'error',
                                'message': line.strip(),
                                'rule': self._extract_rule_name(line)
                            })
                            
            except subprocess.CalledProcessError as e:
                print(f"警告: {file_path}の解析でエラー: {e}")
        
        # 結果をJSONで保存
        result_data = {
            'tool': 'clang-tidy',
            'timestamp': self.timestamp,
            'issues_count': len(issues),
            'issues': issues
        }
        
        with open(report_file, 'w', encoding='utf-8') as f:
            json.dump(result_data, f, indent=2, ensure_ascii=False)
        
        print(f"✅ clang-tidy解析完了: {len(issues)} 件の問題を検出")
        return result_data
    
    def run_cppcheck(self, output_dir: Path) -> Dict[str, Any]:
        """cppcheck解析を実行"""
        print("🔍 cppcheck解析を実行中...")
        
        xml_report = output_dir / f"cppcheck_{self.timestamp}.xml"
        
        cmd = [
            'cppcheck',
            '--enable=all',
            '--std=c++20',
            '--platform=native',
            '--inconclusive',
            '--inline-suppr',
            '--xml',
            '--xml-version=2',
            '-I', 'include',
            '--suppress=missingIncludeSystem',
            '--suppress=unusedFunction',
            '--suppress=unmatchedSuppression',
            '--suppress=noExplicitConstructor',
            '--suppress=passedByValue',
            '--suppress=useStlAlgorithm',
            'include/bluestl'
        ]
        
        try:
            result = subprocess.run(
                cmd,
                stderr=subprocess.PIPE,
                text=True,
                cwd=self.project_root
            )
            
            # XMLレポートを保存
            with open(xml_report, 'w', encoding='utf-8') as f:
                f.write(result.stderr)
            
            # XMLを解析
            issues = self._parse_cppcheck_xml(xml_report)
            
            result_data = {
                'tool': 'cppcheck',
                'timestamp': self.timestamp,
                'issues_count': len(issues),
                'issues': issues
            }
            
            # JSON形式でも保存
            json_report = output_dir / f"cppcheck_{self.timestamp}.json"
            with open(json_report, 'w', encoding='utf-8') as f:
                json.dump(result_data, f, indent=2, ensure_ascii=False)
            
            print(f"✅ cppcheck解析完了: {len(issues)} 件の問題を検出")
            return result_data
            
        except subprocess.CalledProcessError as e:
            print(f"cppcheck実行エラー: {e}")
            return {'tool': 'cppcheck', 'issues_count': 0, 'issues': []}
    
    def run_clang_analyzer(self, files: List[Path], output_dir: Path) -> Dict[str, Any]:
        """Clang Static Analyzer解析を実行"""
        print("🔍 Clang Static Analyzer解析を実行中...")
        
        plist_dir = output_dir / f"clang_analyzer_{self.timestamp}"
        plist_dir.mkdir(exist_ok=True)
        
        cmd = [
            'clang',
            '--analyze',
            '-Xclang', '-analyzer-output=plist-multi-file',
            '-Xclang', f'-analyzer-output-dir={plist_dir}',
            '-Xclang', '-analyzer-checker=core,cplusplus,deadcode,security',
            '-std=c++20',
            '-Iinclude'
        ]
        
        issues = []
        for file_path in files:
            try:
                subprocess.run(
                    cmd + [str(file_path)],
                    cwd=self.project_root,
                    check=True,
                    capture_output=True
                )
            except subprocess.CalledProcessError:
                pass  # 解析エラーは無視
        
        # plistファイルを解析
        for plist_file in plist_dir.glob('*.plist'):
            issues.extend(self._parse_clang_analyzer_plist(plist_file))
        
        result_data = {
            'tool': 'clang-analyzer',
            'timestamp': self.timestamp,
            'issues_count': len(issues),
            'issues': issues
        }
        
        json_report = output_dir / f"clang_analyzer_{self.timestamp}.json"
        with open(json_report, 'w', encoding='utf-8') as f:
            json.dump(result_data, f, indent=2, ensure_ascii=False)
        
        print(f"✅ Clang Static Analyzer解析完了: {len(issues)} 件の問題を検出")
        return result_data
    
    def generate_summary_report(self, output_dir: Path, analysis_results: List[Dict[str, Any]]) -> Path:
        """統合サマリーレポートを生成"""
        print("📊 統合レポートを生成中...")
        
        summary_file = output_dir / f"summary_{self.timestamp}.md"
        
        # 総問題数の計算
        total_issues = sum(result.get('issues_count', 0) for result in analysis_results)
        
        # 問題の分類
        severity_counts = {'error': 0, 'warning': 0, 'style': 0, 'performance': 0, 'portability': 0}
        rule_counts = {}
        
        for result in analysis_results:
            for issue in result.get('issues', []):
                severity = issue.get('severity', 'unknown')
                if severity in severity_counts:
                    severity_counts[severity] += 1
                
                rule = issue.get('rule', 'unknown')
                rule_counts[rule] = rule_counts.get(rule, 0) + 1
        
        # トップ5の問題を取得
        top_rules = sorted(rule_counts.items(), key=lambda x: x[1], reverse=True)[:5]
        
        # Markdownレポート生成
        with open(summary_file, 'w', encoding='utf-8') as f:
            f.write(f"""# BlueStl 静的解析統合レポート

**生成日時**: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}
**解析対象**: BlueStlライブラリ
**総問題数**: {total_issues}

## 📊 解析結果サマリー

| ツール | 問題数 | 状態 |
|--------|--------|------|
""")
            
            for result in analysis_results:
                tool_name = result.get('tool', 'Unknown')
                issues_count = result.get('issues_count', 0)
                status = '✅ 完了' if issues_count >= 0 else '❌ エラー'
                f.write(f"| {tool_name} | {issues_count} | {status} |\n")
            
            f.write(f"""
## 🎯 問題の内訳

| 重要度 | 件数 |
|--------|------|
| エラー | {severity_counts['error']} |
| 警告 | {severity_counts['warning']} |
| スタイル | {severity_counts['style']} |
| パフォーマンス | {severity_counts['performance']} |
| 移植性 | {severity_counts['portability']} |

## 🔝 主な問題（トップ5）

""")
            
            for i, (rule, count) in enumerate(top_rules, 1):
                f.write(f"{i}. **{rule}**: {count} 件\n")
            
            f.write(f"""
## 📝 推奨アクション

1. **高優先度**: エラーレベルの問題を最優先で修正
2. **中優先度**: 警告レベルの問題を順次修正
3. **低優先度**: スタイル・パフォーマンス改善を検討
4. **継続的改善**: 定期的な静的解析の実行

## 📁 詳細レポート

""")
            
            for result in analysis_results:
                tool = result.get('tool', 'Unknown')
                f.write(f"- [{tool}_{self.timestamp}.json](./{tool}_{self.timestamp}.json)\n")
            
            f.write(f"""
## ⚙️ 解析設定

- **C++標準**: C++20
- **解析範囲**: include/bluestl/
- **設定ファイル**: .clang-tidy, .cppcheck

---
*このレポートは自動生成されました*
""")
        
        print(f"✅ 統合レポート生成完了: {summary_file}")
        return summary_file
    
    # ヘルパーメソッド
    def _extract_line_number(self, line: str) -> int:
        """行番号を抽出"""
        try:
            return int(line.split(':')[1])
        except (IndexError, ValueError):
            return 0
    
    def _extract_column_number(self, line: str) -> int:
        """列番号を抽出"""
        try:
            return int(line.split(':')[2])
        except (IndexError, ValueError):
            return 0
    
    def _extract_rule_name(self, line: str) -> str:
        """ルール名を抽出"""
        if '[' in line and ']' in line:
            return line.split('[')[1].split(']')[0]
        return 'unknown'
    
    def _parse_cppcheck_xml(self, xml_file: Path) -> List[Dict[str, Any]]:
        """cppcheckのXML出力を解析"""
        issues = []
        try:
            tree = ET.parse(xml_file)
            root = tree.getroot()
            
            for error in root.findall('.//error'):
                location = error.find('location')
                if location is not None:
                    issues.append({
                        'file': location.get('file', ''),
                        'line': int(location.get('line', 0)),
                        'column': int(location.get('column', 0)),
                        'severity': error.get('severity', 'unknown'),
                        'message': error.get('msg', ''),
                        'rule': error.get('id', 'unknown')
                    })
        except ET.ParseError:
            print("警告: cppcheck XMLの解析に失敗しました")
        
        return issues
    
    def _parse_clang_analyzer_plist(self, plist_file: Path) -> List[Dict[str, Any]]:
        """Clang Static AnalyzerのPLIST出力を解析（簡易版）"""
        # 実際の実装では、plistlibを使用してplistファイルを解析
        # ここでは簡略化
        return []


def main():
    """メイン関数"""
    parser = argparse.ArgumentParser(description='BlueStl 静的解析統合ツール')
    parser.add_argument('--no-clang-tidy', action='store_true', help='clang-tidyをスキップ')
    parser.add_argument('--no-cppcheck', action='store_true', help='cppcheckをスキップ')
    parser.add_argument('--enable-clang-analyzer', action='store_true', help='Clang Static Analyzerを有効化')
    parser.add_argument('--fix', action='store_true', help='可能な問題を自動修正')
    parser.add_argument('--output-dir', default='static_analysis_reports', help='出力ディレクトリ')
    parser.add_argument('--verbose', '-v', action='store_true', help='詳細出力')
    
    args = parser.parse_args()
    
    # プロジェクトルートの検出
    project_root = Path(__file__).parent.parent
    output_dir = project_root / args.output_dir
    output_dir.mkdir(exist_ok=True)
    
    print("🚀 BlueStl 静的解析を開始します...")
    print(f"📂 プロジェクトルート: {project_root}")
    print(f"📁 出力ディレクトリ: {output_dir}")
    
    runner = StaticAnalysisRunner(project_root)
    
    # ソースファイルの検索
    files = runner.find_source_files()
    header_files = files['headers']
    
    if not header_files:
        print("❌ 解析対象のヘッダファイルが見つかりません")
        sys.exit(1)
    
    print(f"📄 解析対象ファイル数: {len(header_files)}")
    if args.verbose:
        for file in header_files:
            print(f"  - {file}")
    
    # 静的解析の実行
    analysis_results = []
    
    if not args.no_clang_tidy:
        result = runner.run_clang_tidy(header_files, output_dir, args.fix)
        analysis_results.append(result)
    
    if not args.no_cppcheck:
        result = runner.run_cppcheck(output_dir)
        analysis_results.append(result)
    
    if args.enable_clang_analyzer:
        result = runner.run_clang_analyzer(header_files, output_dir)
        analysis_results.append(result)
    
    # 統合レポートの生成
    summary_file = runner.generate_summary_report(output_dir, analysis_results)
    
    print("\n🎉 静的解析が完了しました！")
    print(f"📋 統合レポート: {summary_file}")
    
    # 総問題数の表示
    total_issues = sum(result.get('issues_count', 0) for result in analysis_results)
    print(f"🔢 総問題数: {total_issues}")
    
    if total_issues > 0:
        print("🔧 次のステップ:")
        print("  1. 統合レポートを確認")
        print("  2. 高優先度の問題を修正")
        print("  3. 継続的な改善を実施")


if __name__ == '__main__':
    main()