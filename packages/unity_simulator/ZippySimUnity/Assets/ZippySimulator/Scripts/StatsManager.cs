using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using Zippy.Interop;

namespace Zippy {
    public class StatsManager : SingletonBase<StatsManager> {
        bool m_reportStats = false;
        SimulatorStats m_stats;

        // Use this for initialization
        void Start () {
            CommandManager.Instance.OnEnableStats += HandleEnableStats;
        }

        // Update is called once per frame
        void Update () {
            if (m_reportStats) {
                m_stats.deltaTimeMs = Time.deltaTime * 1000.0f;
                m_stats.frameNumber = Time.frameCount;
                SimulatorNetworkManager.SendStats (m_stats);
            }
        }

        void HandleEnableStats(bool enable) {
            m_reportStats = enable;
        }
    }
}