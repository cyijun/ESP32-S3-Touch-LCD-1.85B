#!/usr/bin/env python3
"""
查询 Kimi For Coding 用量进度

用法:
    python kimi_usage.py <api_key>
    或
    export KIMI_API_KEY="your-api-key"
    python kimi_usage.py
"""

import sys
import os
import json
import math
from datetime import datetime, timezone

import requests

API_URL = "https://api.kimi.com/coding/v1/usages"
TIMEOUT = 10


def parse_f64(value) -> float | None:
    """解析数值，兼容 int/float 和字符串格式。"""
    if value is None:
        return None
    if isinstance(value, (int, float)):
        return float(value)
    if isinstance(value, str):
        try:
            return float(value)
        except ValueError:
            return None
    return None


def extract_reset_time(value) -> str | None:
    """提取重置时间，兼容 ISO 8601 字符串和秒/毫秒时间戳。"""
    if value is None:
        return None
    if isinstance(value, str):
        return value
    if isinstance(value, (int, float)):
        n = int(value)
        # 区分秒和毫秒：秒级时间戳 < 1e12，毫秒 >= 1e12
        if n < 1_000_000_000_000:
            n = n * 1000
        # 毫秒转 ISO 8601
        try:
            dt = datetime.fromtimestamp(n / 1000.0, tz=timezone.utc)
            return dt.isoformat()
        except (OSError, ValueError, OverflowError):
            return None
    return None


def format_percentage(val: float) -> str:
    if val == 0.0:
        return "0%"
    if val == 100.0:
        return "100%"
    return f"{val:.1f}%"


def format_time_left(iso_str: str | None) -> str:
    """计算距离重置时间还剩多久。"""
    if not iso_str:
        return "未知"
    try:
        # 兼容带 Z 和 +00:00 的格式
        dt_str = iso_str.replace("Z", "+00:00")
        dt = datetime.fromisoformat(dt_str)
        now = datetime.now(timezone.utc)
        if dt.tzinfo is None:
            dt = dt.replace(tzinfo=timezone.utc)
        delta = dt - now
        if delta.total_seconds() <= 0:
            return "已重置"
        hours, remainder = divmod(int(delta.total_seconds()), 3600)
        minutes, seconds = divmod(remainder, 60)
        if hours > 0:
            return f"{hours}小时{minutes}分钟"
        elif minutes > 0:
            return f"{minutes}分钟{seconds}秒"
        else:
            return f"{seconds}秒"
    except Exception:
        return "未知"


def query_kimi(api_key: str) -> dict:
    headers = {
        "Authorization": f"Bearer {api_key}",
        "Accept": "application/json",
    }

    try:
        resp = requests.get(API_URL, headers=headers, timeout=TIMEOUT)
    except requests.RequestException as e:
        return {
            "success": False,
            "error": f"Network error: {e}",
            "tiers": [],
        }

    if resp.status_code in (401, 403):
        return {
            "success": False,
            "error": f"Authentication failed (HTTP {resp.status_code}): API Key 无效或已过期",
            "tiers": [],
        }

    if not resp.ok:
        body = resp.text
        return {
            "success": False,
            "error": f"API error (HTTP {resp.status_code}): {body}",
            "tiers": [],
        }

    try:
        body = resp.json()
    except json.JSONDecodeError as e:
        return {
            "success": False,
            "error": f"Failed to parse response: {e}",
            "tiers": [],
        }

    tiers = []

    # 1) 5 小时窗口限额（limits[].detail）
    limits = body.get("limits")
    if isinstance(limits, list):
        for limit_item in limits:
            detail = limit_item.get("detail") if isinstance(limit_item, dict) else None
            if isinstance(detail, dict):
                limit = parse_f64(detail.get("limit")) or 1.0
                remaining = parse_f64(detail.get("remaining")) or 0.0
                resets_at = extract_reset_time(detail.get("resetTime"))

                used = max(limit - remaining, 0.0)
                utilization = (used / limit * 100.0) if limit > 0.0 else 0.0
                tiers.append({
                    "name": "five_hour",
                    "label": "5小时窗口",
                    "limit": limit,
                    "remaining": remaining,
                    "used": used,
                    "utilization": utilization,
                    "resets_at": resets_at,
                })

    # 2) 总体用量 / 周限额（usage）
    usage = body.get("usage")
    if isinstance(usage, dict):
        limit = parse_f64(usage.get("limit")) or 1.0
        remaining = parse_f64(usage.get("remaining")) or 0.0
        resets_at = extract_reset_time(usage.get("resetTime"))

        used = max(limit - remaining, 0.0)
        utilization = (used / limit * 100.0) if limit > 0.0 else 0.0
        tiers.append({
            "name": "weekly_limit",
            "label": "周限额",
            "limit": limit,
            "remaining": remaining,
            "used": used,
            "utilization": utilization,
            "resets_at": resets_at,
        })

    return {
        "success": True,
        "error": None,
        "tiers": tiers,
    }


def print_result(result: dict) -> None:
    if not result["success"]:
        print(f"查询失败: {result['error']}")
        sys.exit(1)

    tiers = result["tiers"]
    if not tiers:
        print("未获取到用量信息。")
        return

    print("=" * 50)
    print("Kimi For Coding 用量进度")
    print("=" * 50)
    for tier in tiers:
        name = tier["label"]
        used = tier["used"]
        limit = tier["limit"]
        remaining = tier["remaining"]
        utilization = tier["utilization"]
        resets_at = tier["resets_at"]
        time_left = format_time_left(resets_at)

        # 绘制简单的进度条
        bar_len = 20
        filled = min(math.ceil(utilization / 100.0 * bar_len), bar_len)
        bar = "█" * filled + "░" * (bar_len - filled)

        print(f"\n[{name}]")
        print(f"  {bar} {format_percentage(utilization)}")
        print(f"  已用: {used:,.0f} / 总额: {limit:,.0f} (剩余: {remaining:,.0f})")
        print(f"  重置时间: {resets_at or '未知'}")
        print(f"  距重置: {time_left}")
    print("=" * 50)


def main():
    api_key = None
    if len(sys.argv) > 1:
        api_key = sys.argv[1]
    else:
        api_key = os.environ.get("KIMI_API_KEY")

    if not api_key:
        print("Usage: python kimi_usage.py <api_key>")
        print("   or: export KIMI_API_KEY=<api_key> && python kimi_usage.py")
        sys.exit(1)

    result = query_kimi(api_key)
    print_result(result)


if __name__ == "__main__":
    main()
