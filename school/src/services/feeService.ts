import { api } from './api';
import type { Fee, Payment } from '@/types';

export const feeService = {
  async getAll(): Promise<Fee[]> {
    return api.get<Fee[]>('/fees');
  },

  async getByStudent(studentId: string): Promise<Fee[]> {
    return api.get<Fee[]>(`/fees?studentId=${studentId}`);
  },

  async getById(id: string): Promise<Fee> {
    return api.get<Fee>(`/fees/${id}`);
  },

  async create(fee: Omit<Fee, 'id'>): Promise<Fee> {
    return api.post<Fee>('/fees', fee);
  },

  async update(id: string, fee: Partial<Fee>): Promise<Fee> {
    return api.patch<Fee>(`/fees/${id}`, fee);
  },

  async delete(id: string): Promise<void> {
    return api.delete(`/fees/${id}`);
  },

  async recordPayment(feeId: string, payment: Omit<Payment, 'id' | 'feeId'>): Promise<Payment> {
    return api.post<Payment>(`/fees/${feeId}/payments`, payment);
  },

  async getPaymentHistory(feeId: string): Promise<Payment[]> {
    return api.get<Payment[]>(`/fees/${feeId}/payments`);
  },

  async getUnpaidFees(): Promise<Fee[]> {
    return api.get<Fee[]>('/fees?status=unpaid');
  },

  async getStats(): Promise<{
    totalDue: number;
    totalPaid: number;
    totalUnpaid: number;
    unpaidCount: number;
  }> {
    return api.get('/fees/stats');
  },
};
