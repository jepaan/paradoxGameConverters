﻿using Caliburn.Micro;
using Frontend.Core.Logging;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Threading;

namespace Frontend.Core.Converting.Operations
{
    public class OperationProcessor : DispatcherObject, IOperationProcessor
    {
        private IEventAggregator eventAggregator;
        private Dictionary<OperationResultState, Action<OperationResult, IOperationViewModel>> resultHandlershandle;

        public OperationProcessor(IEventAggregator eventAggregator)
        {
            this.eventAggregator = eventAggregator;
            this.resultHandlershandle = new Dictionary<OperationResultState, Action<OperationResult, IOperationViewModel>>
            {
                { OperationResultState.Success, this.HandleSuccess },
                { OperationResultState.Warning, this.HandleWarning },
                { OperationResultState.Error, this.HandleErrors }
            };
        }

        public async Task<int> ProcessQueue(IEnumerable<IOperationViewModel> operations, IProgress<int> progress, CancellationToken token)
        {
            int totalCount = operations.Count();
            int processCount = await Task.Run<int>(() =>
                {
                    int currentCount = 0;

                    foreach (var operation in operations)
                    {
                        currentCount++;

                        operation.State = OperationState.InProgress;
                        this.eventAggregator.PublishOnUIThread(new LogEntry(operation.Description, LogEntrySeverity.Info, LogEntrySource.UI));

                        var dummyResult = new OperationResult();
                        Task<OperationResult> task = Task.FromResult(dummyResult);

                        try
                        {
                            task = operation.Process();
                            token.ThrowIfCancellationRequested();

                            this.LogResultMessages(task.Result);
                            this.resultHandlershandle[task.Result.State](task.Result, operation);

                            this.ReportProgress(progress, currentCount, totalCount);
                        }
                        catch (OperationCanceledException oce)
                        {
                            this.HandleCancellation(task.Result, operation, operations, progress);
                            break;
                        }
                        catch (Exception e)
                        {
                            this.eventAggregator.PublishOnUIThread(new LogEntry(string.Format("Unhandled exception occurred: {0}", e.Message), LogEntrySeverity.Error, LogEntrySource.UI));
                        }
                    }

                    return currentCount;
                }, token);

            return processCount;
        }

        private void ReportProgress(IProgress<int> progress, int currentCount, int totalCount)
        {
            if (progress != null)
            {
                int progressValue = currentCount * 100 / totalCount;
                progress.Report(progressValue);
            }
        }

        private void HandleCancellation(OperationResult result, IOperationViewModel operation, IEnumerable<IOperationViewModel> operations, IProgress<int> progressIndicator)
        {
            this.eventAggregator.PublishOnUIThread(new LogEntry(string.Format("Operation {0} cancelled", operation.Description), LogEntrySeverity.Warning, LogEntrySource.UI));

            operations.Where(o => o.State == OperationState.InProgress || o.State == OperationState.NotStarted)
                                .ForEach(o => o.State = OperationState.Cancelled);
            progressIndicator.Report(100);
        }

        private void HandleSuccess(OperationResult result, IOperationViewModel operation)
        {
            operation.State = OperationState.Success;
            this.eventAggregator.PublishOnUIThread(new LogEntry(string.Format("{0} completed successfully.", operation.Description), LogEntrySeverity.Info, LogEntrySource.UI));
        }

        private void HandleWarning(OperationResult result, IOperationViewModel operation)
        {
            operation.State = OperationState.CompleteWithWarnings;
            this.eventAggregator.PublishOnUIThread(new LogEntry(string.Format("{0} completed with warnings!", operation.Description), LogEntrySeverity.Warning, LogEntrySource.UI));
        }

        private void HandleErrors(OperationResult result, IOperationViewModel operation)
        {
            operation.State = OperationState.CompleteWithErrors;
            this.eventAggregator.PublishOnUIThread(new LogEntry(string.Format("{0} failed!", operation.Description), LogEntrySeverity.Error, LogEntrySource.UI));
        }

        private void LogResultMessages(OperationResult result)
        {
            foreach (var logEntry in result.LogEntries)
            {
                this.eventAggregator.PublishOnUIThread(logEntry);
            }
        }

        /// <summary>
        /// Marshalls the method.
        /// </summary>
        /// <param name="action">The action.</param>
        /// <param name="priority">The priority.</param>
        private void MarshallMethod(System.Action action, DispatcherPriority priority)
        {
            if (!this.Dispatcher.CheckAccess())
            {
                this.Dispatcher.Invoke(action, priority);
                return;
            }

            action();
        }
    }
}
